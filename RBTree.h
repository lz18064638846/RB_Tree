#include "RBTreeNode.h"
#include <functional>

namespace rbtree {

template<class KeyType, class ValType>
class RBTree {
public:
    using TreeNode = RBTreeNode<KeyType, ValType>;
    using Compare = std::function<bool(const KeyType&, const KeyType&)>;
    using TreeNode::Color::BLACK;
    using TreeNode::Color::RED;

    class Iterator {
    public:
        Iterator(TreeNode* node) : node_(node) {}
        
        TreeNode* operator->() const { return node_; }
        Iterator& operator++() { node_ = node_->FindNext(); return *this; }
        Iterator& operator++(int) { Iterator tmp = *this; node_ = node_->FindNext(); return tmp; }
        Iterator& operator--() { node_ = node_->FindPrev(); return *this; }
        Iterator& operator--(int) { Iterator tmp = *this; node_ = node_->FindNext(); return tmp; }
        TreeNode& operator*() { return *node_; }
        bool operator==(const Iterator& other) { return this->node_ == other.node_; }
        
    private:
        TreeNode* node_;    
    };

    explicit RBTree(TreeNode* root = nullptr, Compare&& comp = std::less<KeyType>());

    void Put(const KeyType& key, const ValType& val);
    bool Delete(const KeyType& key);
    [[nodiscard]] ValType& Get(const KeyType& key) const;
    size_t Size() const { return size_; }
    bool Empty() const { return size_ == 0; }
    Iterator begin() const { if(!root_) return Iterator(nullptr); auto ret = root_; while(ret->Left()) ret = ret->Left(); return Iterator(ret); }
    Iterator end() const { return Iterator(nullptr); }
    Iterator find(const KeyType& key) const;

    ValType& operator[](const KeyType& key) const { return Get(key); }

    ~RBTree();
private:
    TreeNode* root_;
    Compare compare_;
    size_t size_;

    void InsertFixUp(TreeNode* node);
    void DeleteFixUp(TreeNode* node);
    void LeftRotate(TreeNode* node);
    void RightRotate(TreeNode* node);
    ValType& PutInner(const KeyType& key, const ValType& val);
};

template<class KeyType, class ValType>
RBTree<KeyType, ValType>::RBTree(TreeNode* root, Compare&& comp)
  : root_(root), compare_(comp) {}

template<class KeyType, class ValType>
typename RBTree<KeyType, ValType>::Iterator RBTree<KeyType, ValType>::find(const KeyType& key) const
{
    TreeNode* father = nullptr, *cur = root_;
    while (cur != nullptr) {
        if(compare_(cur->Key(), key)) {
            father = cur;
            cur = cur->Left();
        } else if (compare_(key, cur->Key())) {
            father = cur;
            cur = cur->Right();
        } else {
            break;
        }
    }

    return Iterator(cur);
}

template<class KeyType, class ValType>
bool RBTree<KeyType, ValType>::Delete(const KeyType& key)
{
    size_--;
    TreeNode* father = nullptr, *cur = root_;
    while (cur != nullptr) {
        if(compare_(cur->Key(), key)) {
            father = cur;
            cur = cur->Left();
        } else if (compare_(key, cur->Key())) {
            father = cur;
            cur = cur->Right();
        } else {
            break;
        }
    }
    // Key not found
    if (cur == nullptr) {
        size_++;
        return false;
    }

    auto next = cur->GetSuccessor();
    TreeNode* replaced = next->Left() ? next->Left() : next->Right();
    father = next->Father();
    if(!father) {
        root_ = replaced;
        if (replaced) replaced->SetFather(nullptr);
    } else if (next == father->Left()) {
        father->SetLeft(replaced);
    } else {
        father->SetRight(replaced);
    }
    if (next != cur) {
        cur->Copy(next);
    }
    if (!next->Red()) {
        DeleteFixUp(next);
    }
    next->SetLeft(nullptr);
    next->SetRight(nullptr);
    delete next;
    return true;
}

template<class KeyType, class ValType>
void RBTree<KeyType, ValType>::Put(const KeyType& key, const ValType& val)
{
    PutInner(key, val);
}

template<class KeyType, class ValType>
ValType& RBTree<KeyType, ValType>::PutInner(const KeyType& key, const ValType& val)
{
    size_++;
    if(!root_) {
        root_ = new TreeNode(key, val);
        root_->SetColor(TreeNode::Color::BLACK);
        return root_->Value();
    }

    TreeNode* father = nullptr, *cur = root_;
    while (cur != nullptr) {
        father = cur;
        if(compare_(father->Key(), key)) {
            cur = cur->Left();
        } else if (compare_(key, father->Key())) {
            cur = cur->Right();
        } else {
            size_--;
            father->SetValue(val);
            return father->Value();
        }
    }

    cur = new TreeNode(key, val);

    if(compare_(father->Key(), key)) {
        father->SetLeft(cur);
    } else {
        father->SetRight(cur);
    }

    InsertFixUp(cur);
    return cur->Value();
}

template<class KeyType, class ValType>
void RBTree<KeyType, ValType>::DeleteFixUp(TreeNode* node)
{
    while (node != root_ && !node->Red()) {
        auto father = node->Father();
        if (node == father->Left()) {
            auto sibling = father->Right();
            if (sibling->Red()) {
                LeftRotate(father);
                father->SetColor(RED);
                sibling->SetColor(BLACK);
            }
            // sibling is black 
            else {
                auto right_nephew = sibling->Right(), left_nephew = sibling->Left();
                // Right nephew is red, left rotate the father
                if (right_nephew && right_nephew->Red()) {
                    LeftRotate(father);
                    father->SetColor(BLACK);
                    right_nephew->SetColor(BLACK);
                    sibling->SetColor(RED);
                    break;
                }
                // Left nephew is red, right rotate the nephew to make the right nephew red
                else if(left_nephew && left_nephew->Red()) {
                    RightRotate(left_nephew);
                    left_nephew->SetColor(BLACK);
                    sibling->SetColor(RED);
                }
                // both nephews are black, set father to the new node and handle it
                else {
                    sibling->SetColor(RED);
                    node = father;
                }
            }
        }
    }
    node->SetColor(BLACK);
}

template<class KeyType, class ValType>
void RBTree<KeyType, ValType>::InsertFixUp(TreeNode* node)
{
    TreeNode* cur = node;
    while (true) {
        auto father = cur->Father();
        // cur is root, set it black and return;
        if (!father) {
            cur->SetColor(BLACK);
            return;
        }

        // Father is black, return
        if(!father->Red()) return;

        auto grand_father = father->Father();
        // Father is root, simply return
        if (!grand_father) {
            return;
        }

        // Father is the left son of grand father
        if (grand_father->Left() == father) {
            auto uncle = grand_father->Right();
            // Uncle is red, makes granfather red and uncle black
            if (uncle && uncle->Red()) {
                father->SetColor(BLACK);
                uncle->SetColor(BLACK);
                grand_father->SetColor(RED);
                cur = grand_father;
            } else {
                if (father->Right() == cur) { // cur is right son, rotate left the father
                    LeftRotate(father);
                    cur = father;
                } else { // cur is left son, can't fix it in single rotate, rotate right the granfather
                    RightRotate(grand_father);
                    grand_father->SetColor(RED);
                    father->SetColor(BLACK);
                    return;
                }
            }
        }
        else {
            auto uncle = grand_father->Left();
            // Uncle is red, makes granfather red and uncle black
            if (uncle && uncle->Red()) {
                father->SetColor(BLACK);
                uncle->SetColor(BLACK);
                grand_father->SetColor(RED);
                cur = grand_father;
            } else {
                if (father->Left() == cur) { // cur is left son, rotate right the father
                    RightRotate(father);
                    cur = father;
                } else { // cur is left son, can't fix it in single rotate, rotate right the granfather
                    LeftRotate(grand_father);
                    grand_father->SetColor(RED);
                    father->SetColor(BLACK);
                    return;
                }
            }
        }
    }
}

/*
 *              rotate x
 *      f                   f
 *      |                   |
 *      x                   y
 *     / \        =>       / \
 *    a   y               x   c
 *       / \             / \
 *      b   c           a   b
 */
template<class KeyType, class ValType>
void RBTree<KeyType, ValType>::LeftRotate(TreeNode* node)
{
    // right <- y, father <- f (may be null)
    auto right = node->Right(), father = node->Father();

    // move b to x->right
    node->SetRight(right->Left());

    // move x to y->left
    right->SetLeft(node);

    // If x is root, set y to new root
    if(!father) {
        root_ = right;
        right->SetFather(nullptr);
    } 
    // If x is f->left, set y to f->left
    else if (father->Left() == node){
        father->SetLeft(right);
    } 
    // Last case, x is f->right
    else {
        father->SetRight(right);
    }
}

/*
 *              rotate x
 *      f                   f
 *      |                   |
 *      x                   y
 *     / \        =>       / \
 *    y   a               b   x
 *   / \                     / \
 *  b   c                   c   a
 */
template<class KeyType, class ValType>
void RBTree<KeyType, ValType>::RightRotate(TreeNode* node)
{
    // left <- y, father <- f (may be null)
    auto left = node->Left(), father = node->Father();

    // move c to x->left
    node->SetLeft(left->Right());

    // move x to y->right
    left->SetRight(node);

    // If x is root, set y to new root
    if(!father) {
        root_ = left;
        left->SetFather(nullptr);
    } 
    // If x is f->left, set y to f->left
    else if (father->Left() == node){
        father->SetLeft(left);
    } 
    // Last case, x is f->right
    else {
        father->SetRight(left);
    }
}

template<class KeyType, class ValType>
ValType& RBTree<KeyType, ValType>::Get(const KeyType& key) const
{
    ValType ret {};
    if(!root_) {
        root_ = new TreeNode(key, ret);
        root_->SetColor(TreeNode::Color::BLACK);
        return root_->Value();
    }

    TreeNode* cur = root_;
    while (cur != nullptr) {
        if(compare_(cur->Key(), key)) {
            cur = cur->Left();
        } else if (compare_(key, cur->Key())) {
            cur = cur->Right();
        } else {
            return cur->Value();
        }
    }

    return PutInner(key, ret);
}

template<class KeyType, class ValType>
RBTree<KeyType, ValType>::~RBTree()
{
    // traverse the tree to delete all nodes
    delete root_;
}


} // namespace rbtree