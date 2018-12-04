////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     Реализация классов красно-черного дерева
/// \author    Sergey Shershakov
/// \version   0.1.0
/// \date      01.05.2017
///            This is a part of the course "Algorithms and Data Structures" 
///            provided by  the School of Software Engineering of the Faculty 
///            of Computer Science at the Higher School of Economics.
///
/// "Реализация" (шаблонов) методов, описанных в файле rbtree.h
///
////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>        // std::invalid_argument


namespace xi {


//==============================================================================
// class RBTree::node
//==============================================================================

template <typename Element, typename Compar >
RBTree<Element, Compar>::Node::~Node()
{
    if (_left)
        delete _left;
    if (_right)
        delete _right;
}

template <typename Element, typename Compar>
typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::Node::setLeft(Node* lf)
{
    // предупреждаем повторное присвоение
    if (_left == lf)
        return nullptr;

    // если новый левый — действительный элемент
    if (lf)
    {
        // если у него был родитель
        if (lf->_parent)
        {
            // ищем у родителя, кем был этот элемент, и вместо него ставим бублик
            if (lf->_parent->_left == lf)
                lf->_parent->_left = nullptr;
            else                                    // доп. не проверяем, что он был правым, иначе нарушение целостности
                lf->_parent->_right = nullptr;      
        }

        // задаем нового родителя
        lf->_parent = this;
    }

    // если у текущего уже был один левый — отменяем его родительскую связь и вернем его
    Node* prevLeft = _left;
    _left = lf;

    if (prevLeft)
        prevLeft->_parent = nullptr;

    return prevLeft;
}


template <typename Element, typename Compar>
typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::Node::setRight(Node* rg)
{
    // предупреждаем повторное присвоение
    if (_right == rg)
        return nullptr;

    // если новый правый — действительный элемент
    if (rg)
    {
        // если у него был родитель
        if (rg->_parent)
        {
            // ищем у родителя, кем был этот элемент, и вместо него ставим бублик
            if (rg->_parent->_left == rg)
                rg->_parent->_left = nullptr;
            else                                    // доп. не проверяем, что он был правым, иначе нарушение целостности
                rg->_parent->_right = nullptr;
        }

        // задаем нового родителя
        rg->_parent = this;
    }

    // если у текущего уже был один левый — отменяем его родительскую связь и вернем его
    Node* prevRight = _right;
    _right = rg;

    if (prevRight)
        prevRight->_parent = nullptr;

    return prevRight;
}


//==============================================================================
// class RBTree
//==============================================================================

template <typename Element, typename Compar >
RBTree<Element, Compar>::RBTree()
{
    _root = nullptr;
    _dumper = nullptr;
}

template <typename Element, typename Compar >
RBTree<Element, Compar>::~RBTree()
{
    // грохаем пока что всех через корень
    if (_root)
        delete _root;
}

template <typename Element, typename Compar >
void RBTree<Element, Compar>::deleteNode(Node* nd)
{
    // если переданный узел не существует, просто ничего не делаем, т.к. в вызывающем проверок нет
    if (nd == nullptr)
        return;

    // потомков убьет в деструкторе
    delete nd;
}

template <typename Element, typename Compar >
void RBTree<Element, Compar>::insert(const Element& key)
{
    // этот метод можно оставить студентам целиком
    Node* newNode = insertNewBstEl(key);

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_BST_INS, this, newNode);

    rebalance(newNode);

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_INSERT, this, newNode);

}

template <typename Element, typename Compar>
const typename RBTree<Element, Compar>::Node* RBTree<Element, Compar>::find(const Element& key)
{
    Node* iter = _root;
    while (iter)
    {
        if(key == iter->_key)
            return iter;
        else if (key < iter->_key)
            iter = iter->_left;
        else
            iter = iter->_right;
    }

    return nullptr;
}

template <typename Element, typename Compar >
typename RBTree<Element, Compar>::Node* 
        RBTree<Element, Compar>::insertNewBstEl(const Element& key)
{
    Node* last = nullptr;
    Node* iter = _root;
    while (iter)
    {
        last = iter;
        if(key < iter->_key)
            iter = iter->_left;
        else
            iter = iter->_right;
    }

    Node* newNode = new Node; // creating new node to store key
    newNode->_parent = last;
    newNode->_key = key;
    newNode->_color = Color::RED; // marking new node red

    if(!last) // if it's first element in tree
        _root = newNode;
    else if (key < last->_key)
        last->_left = newNode;
    else
        last->_right = newNode;

    return newNode;
}

template <typename Element, typename Compar >
typename RBTree<Element, Compar>::Node* 
    RBTree<Element, Compar>::rebalanceDUG(Node* nd)
{
    // попадание в этот метод уже означает, что папа есть (а вот про дедушку пока не известно)
    //...
    bool isLeft = nd->_parent->isLeftChild();
    Node* uncle = nullptr; // для левого случая нужен правый дядя и наоборот.
    if(isLeft)
        uncle = nd->_parent->_parent->_right;
    else
        uncle = nd->_parent->_parent->_left;

    // если дядя такой же красный, как сам нод и его папа...
    if (uncle && uncle->isRed())
    {
        nd->_parent->setBlack(); // color daddy in black
        uncle->setBlack(); // color uncle in black
        nd->_parent->_parent->setRed(); // color grandpa in red
        nd = nd->_parent->_parent;

        // отладочное событие
        if (_dumper)
            _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR1, this, nd);

        return nd;

        // теперь чередование цветов "узел-папа-дедушка-дядя" — К-Ч-К-Ч, но надо разобраться, что там
        // с дедушкой и его предками, поэтому продолжим с дедушкой
        //..
    }

    // дядя черный
    // смотрим, является ли узел "правильно-правым" у папочки

    Node* nearUncle = nullptr;
    if(isLeft)
        nearUncle = nd->_parent->_right;
    else
        nearUncle = nd->_parent->_left;

    if (nearUncle && nd == nearUncle)    // для левого случая нужен правый узел, поэтом отрицание
    {
        nd = nd->_parent;
        if(isLeft)
            rotLeft(nd);
        else
            rotRight(nd);
    }

    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3D, this, nd);

    nd->_parent->setBlack();
    nd->_parent->_parent->setRed();     // деда в красный
    if(isLeft)
        rotRight(nd->_parent->_parent);
    else
        rotLeft(nd->_parent->_parent);

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RECOLOR3G, this, nd);

    return nd;
}

template <typename Element, typename Compar >
void RBTree<Element, Compar>::rebalance(Node* nd)
{
    // пока папа — цвета пионерского галстука, действуем
    while (nd->_parent && nd->_parent->_color == RED)
    {
        nd = rebalanceDUG(nd);
    }

    _root->_color = BLACK; // coloring root as black
}

template <typename Element, typename Compar>
void RBTree<Element, Compar>::rotLeft(typename RBTree<Element, Compar>::Node* nd)
{
    if(!nd) return;

    Node* y = nd->_right; // правый потомок, который станет после левого поворота "выше"

    if (!y)
        throw std::invalid_argument("Can't rotate left since the right child is nil");

    nd->_right = y->_left; // from y left subtree to x right sub tree
    if(y->_left)
        y->_left->_parent = nd;

    y->_parent = nd->_parent; // getting parent of nd
    if(!nd->_parent)
        _root = y;
    else if (nd == nd->_parent->_left)
        nd->_parent->_left = y;
    else
        nd->_parent->_right = y;

    y->_left = nd;
    nd->_parent = y;

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_LROT, this, nd);
}

template <typename Element, typename Compar>
void RBTree<Element, Compar>::rotRight(typename RBTree<Element, Compar>::Node* nd)
{
    if(!nd) return;

    Node* x = nd->_left;

    if (!x)
        throw std::invalid_argument("Can't rotate right since the left child is nil");

    nd->_left = x->_right; // from x right subtree to y left subtree
    if(x->_right)
        x->_right->_parent = nd;

    x->_parent = nd->_parent;
    if(!nd->_parent)
        _root = x;
    else if (nd == nd->_parent->_left)
        nd->_parent->_left = x;
    else if (nd == nd->_parent->_right)
        nd->_parent->_right = x;

    x->_right = nd;
    nd->_parent = x;

    // отладочное событие
    if (_dumper)
        _dumper->rbTreeEvent(IRBTreeDumper<Element, Compar>::DE_AFTER_RROT, this, nd);
}


} // namespace xi

