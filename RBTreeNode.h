
namespace rbtree {
template<class KeyType, class ValType>
class RBTreeNode
{
public:
    enum class Color {
        RED,
        BLACK
    };

    RBTreeNode(RBTreeNode* left = nullptr, RBTreeNode* right = nullptr, RBTreeNode* father = nullptr);
    RBTreeNode(KeyType key, ValType val, RBTreeNode* left = nullptr, RBTreeNode* right = nullptr, RBTreeNode* father = nullptr);
    ~RBTreeNode();

    void SetLeft(RBTreeNode* left_child) { left_child_ = left_child; if(left_child) left_child->father_ = this; }
    void SetRight(RBTreeNode* right_child) { right_child_ = right_child; if(right_child) right_child->father_ = this; }
    void SetFather(RBTreeNode* father) { father_ = father; }
    void SetColor(Color color) { color_ = color; }
    void SetValue(const ValType& val) { val_ = val; }
    
    const KeyType& Key() const { return key_; }
    ValType& Value() { return val_; }
    RBTreeNode* Left() const { return left_child_; }
    RBTreeNode* Right() const { return right_child_; }
    RBTreeNode* Father() const { return father_; }
    RBTreeNode* FindNext() const;
    RBTreeNode* FindPrev() const;
    const std::pair<KeyType, ValType>& Node() const { return {key_, val_}; }
    const bool Red() const { return color_ == Color::RED; }

    RBTreeNode* GetSuccessor() const;
    void Copy(const RBTreeNode* other);

    // RBTreeNode& operator=(const RBTreeNode& other);
    // RBTreeNode& operator=(const RBTreeNode&& other);

    // RBTreeNode(const RBTreeNode& other);
    // RBTreeNode(const RBTreeNode&& other);

private:
    KeyType key_;
    ValType val_;
    RBTreeNode* left_child_;
    RBTreeNode* right_child_;
    RBTreeNode* father_;
    Color color_;
};

template<class KeyType, class ValType>
RBTreeNode<KeyType, ValType>::RBTreeNode(RBTreeNode* left, RBTreeNode* right, RBTreeNode* father)
  : left_child_(left), right_child_(right), father_(father), color_(Color::RED) {}

template<class KeyType, class ValType>
RBTreeNode<KeyType, ValType>::RBTreeNode(KeyType key, ValType val, RBTreeNode* left, RBTreeNode* right, RBTreeNode* father)
  : key_(key), val_(val), left_child_(left), right_child_(right), father_(father), color_(Color::RED) {}

template<class KeyType, class ValType>
RBTreeNode<KeyType, ValType>* RBTreeNode<KeyType, ValType>::GetSuccessor() const
{
  if (!left_child_ || !right_child_) return this;

  auto cur = left_child_;
  while (cur->right_child_) {
      cur = cur->right_child_;
  }
  return cur;
}

template<class KeyType, class ValType>
void RBTreeNode<KeyType, ValType>::Copy(const RBTreeNode* other)
{
  key_ = other->key_;
  val_ = other->val_;
}

template<class KeyType, class ValType>
RBTreeNode<KeyType, ValType>* RBTreeNode<KeyType, ValType>::FindNext() const
{
  if (right_child_) {
    auto ret = right_child_;
    while (ret->left_child_) {
      ret = ret->left_child_;
    }
    return ret;
  }
  auto ret = father_, cur = this;
  while (ret && ret->right_child_ == cur) {
      cur = ret;
      ret = ret->father_;
  }
  return nullptr;
}

template<class KeyType, class ValType>
RBTreeNode<KeyType, ValType>* RBTreeNode<KeyType, ValType>::FindPrev() const
{
  if (left_child_) {
    auto ret = left_child_;
    while (ret->right_child_) {
      ret = ret->right_child_;
    }
    return ret;
  }
  auto ret = father_, cur = this;
  while (ret && ret->left_child_ == cur) {
      cur = ret;
      ret = ret->father_;
  }
  return nullptr;
}

template<class KeyType, class ValType>
RBTreeNode<KeyType, ValType>::~RBTreeNode()
{
  delete left_child_;
  delete right_child_;
}

} // namespace rbtree