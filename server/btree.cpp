#include "btree.h"

/**
 * Constructor of Btree class.
 * @param _root - the root of the btree.
 * @param _depth - depth of the btree.
 * @param _r_node - the right most index node.
 * @param _out - the index output file.
 */
Btree::Btree(std::fstream& out) {
    _root = new index_node_t;
    _depth = 1;
    _r_node = _root;
    this->write_index_node(_root);
}

/**
 * Deconstructor of Btree class.
 * delete all its pointers.
 */
Btree::~Btree() {
    delete _root;
    delete _r_node;
    _out.close();
}

/**
 * write an index node into the disk.
 * @param offset - the offset of the inserted index node in disk.
 */
void Btree::write_index_node(index_node_t*& index_node)
{
    _ptr_t offset(_out.tellp() >> PAGE_OFFSET);
    index_node->offset = offset;
    _out.write(reinterpret_cast<char*>(index_node), PAGE_SIZE);
}

/**
 * update the index node in the disk.
 */
void Btree::update_index_node(const index_node_t*& index_node)
{
    long current = _out.tellp();
    _out.seekp(index_node->offset.to_ulong());
    _out.write(reinterpret_cast<char*>(index_node), PAGE_SIZE);
    _out.seekp(current);
}

/**
 * create the b+ btree using bulk loading.
 * @param leaf_node_list the leaf nodes.
 */
void Btree::bulk_load(std::vector<leaf_node_t*> leaf_node_list)
{
    typedef typename std::vector<leaf_node_t*>::iterator iter_t;
    iter_t iter;
    for (iter = leaf_node_list.begin();
         iter != leaf_node_list.end(); ++iter) {
        this->insert(*iter);
    }
}

/**
 * insert a leaf node into the btree.
 */
void Btree::insert(const leaf_node_t* leaf_node)
{
    _ptr_t offset(_out.tellp()); // offset -- the current position
    _out.write(reinterpret_cast<char*>(leaf_node), PAGE_SIZE);

    size_t cnt = _r_node->cnt;
    if (cnt > _FANOUT) {
        this->copy_up(leaf_node);
    } else if (cnt == 0) {
        _r_node->ptr = offset;
        _r_node->cnt++;
        this->update_index_node(_r_node);
    } else { // directly insert
        index_node_t* index_node = &_r_node->records[cnt-1];
        index_node->key = leaf_node->records[0].key;
        index_node->ptr = offset;
        _r_node->cnt++;
        this->update_index_node(_r_node);
    }
}

/**
 * copy up one record to its upper index node.
 */
void Btree::copy_up(const leaf_node_t*& leaf_node, _ptr_t offset)
{
    record_t* copy_up_record = new record_t;
    copy_up_record->key = leaf_node->records[0].kconstey;
    copy_up_record->ptr = offset;
    this->split(copy_up_record);
}

/**
 * split the relative index node.
 */
void Btree::split(record_t*& copy_up_record)
{
    index_node_t* l_node = _r_node;

    /*step 1: new a right node*/
    _r_node = new index_node_t;

    /*step 2: copy the rightmost ptr of l_node to the ptr of r_node*/
    _r_node->records[0].ptr = l_node->records[l_node->cnt-2].ptr;
    _r_node->cnt++;

    /*step 3: put the copy_up_record into the r_node*/
    _r_node->records[0].key = copy_up_record->key;
    _r_node->records[0].ptr = copy_up_record->ptr;
    _r_node->cnt++;

    /*step 4: set the parent of r_node*/
    _r_node->parent = l_node->parent;
    this->write_index_node(_r_node);

    /*step 5: push up the rightmost element of l_node*/
    _key_t push_up_elem = l_node->records[l_node->cnt-2];
    l_node->cnt--;
    this->update_index_page(l_node);
    this->btree_push_up(_r_node->parent, push_up_elem);
}

/**
 * push up one record.
 */
void Btree::push_up(_ptr_t& parent_ptr, _key_t& push_up_elem, index_node_t*& l_node)
{

    if (parent_ptr.to_ulong() == 0) {
        _root = new index_node_t;
        _depth++;

        /*set the left and right pointer*/
        _root->cnt++;
        _root->ptr = l_node->offset;
        _root->records[0].key = push_up_elem;
        _root->records[0].ptr = _r_node->offset;
        _root->cnt++;
        this->write_index_node(_root);

        _r_node->parent = l_node->parent = _root->offset;
    }
    else {
        long current = _out.tellp();
        long offset = parent_ptr.to_ulong() << PAGE_OFFSET;
        _out.seekp(offset);

        index_node_t* parent = new index_node_t;
        _out.read(parent, sizeof(index_node_t));
        _out.seekp(current);

        if (parent->cnt > _FANOUT) {
            /*step 1: new a rightmost page*/
            index_node_t* r_parent;
            index_node_t* l_parent = parent;

            r_parent = new index_node_t;

            /*step 2: copy the ptr of the l_parent and the push up element into r_parent*/
            r_parent->ptr = l_parent->records[l_parent->cnt-2].ptr;
            r_parent->cnt++;
            r_parent->records[0].key = push_up_elem;
            r_parent->records[0].ptr = _r_node->offset;
            r_parent->cnt++;
            r_parent->parent = l_parent->parent;

            /*step 3: change the parent of l_node and r_node*/
            l_node->parent = r_parent->offset;
            _r_node->parent = r_parent->offset;

            /*push up the key in the rightmost position of the l_parent*/
            push_up_elem = l_parent->records[l_parent->cnt-2].key;
            l_parent->cnt--;

            // update the changed nodes in the disk
            this->write_index_node(r_parent);
            this->update_index_node(l_parent);
            this->update_index_node(l_node);
            this->update_index_node(_r_node);

            this->push_up(parent->parent, push_up_elem, l_parent);
        }
        else {
            parent->records[parent->cnt-1].key = push_up_elem;
            parent->records[parent->cnt-1].ptr = _r_node->offset;
            parent->cnt++;

            this->update_index_node(parent);

            if (parent->offset = _root->offset) {
                long current = _out.tellp();
                long offset = _root.to_ulong() << PAGE_OFFSET;
                _out.seekp(offset);
                _out.read(_root, sizeof(index_node_t));
                _out.seekp(current);
            }
        }
    }
}

/**
 * print the tree structure.
 */
void Btree::print_info()
{

}
