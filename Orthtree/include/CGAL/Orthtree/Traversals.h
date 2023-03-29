// Copyright (c) 2007-2020  INRIA (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Jackson Campolattaro, Cédric Portaneri, Tong Zhao

#ifndef CGAL_ORTHTREE_TRAVERSALS_H
#define CGAL_ORTHTREE_TRAVERSALS_H

#include <CGAL/license/Orthtree.h>

#include <iostream>
#include <boost/range/iterator_range.hpp>
#include <CGAL/Orthtree/Traversal_iterator.h>

#include <stack>

namespace CGAL {

/// \cond SKIP_IN_MANUAL
// todo: is this necessary?
// Forward declaration
template <typename T, typename PR, typename PM>
class Orthtree;
/// \endcond

namespace Orthtrees {

/*!
  \ingroup PkgOrthtreeTraversal
  \brief A class used for performing a preorder traversal.

  A preorder traversal starts from the root towards the leaves.

  \cgalModels `OrthtreeTraversal`
 */
template <typename Tree>
struct Preorder_traversal {
private:

  using Node = typename Tree::Node;

  const Tree& m_orthtree;

public:

  Preorder_traversal(const Tree& orthtree) : m_orthtree(orthtree) {}

  const Node* first() const {
    return &m_orthtree.root();
  }

  const Node* next(const Node* n) const {

    if (n->is_leaf()) {

      auto next = m_orthtree.next_sibling(n);

      if (nullptr == next) {

        return m_orthtree.next_sibling_up(n);
      }

      return next;

    } else {

      // Return the first child of this node
      return &m_orthtree.children(*n)[0];
    }

  }
};

/*!
  \ingroup PkgOrthtreeTraversal
  \brief A class used for performing a postorder traversal.

  A postorder traversal starts from the leaves towards the root.

  \cgalModels `OrthtreeTraversal`
 */
template <typename Tree>
struct Postorder_traversal {
private:

  using Node = typename Tree::Node;

  const Tree& m_orthtree;

public:

  Postorder_traversal(const Tree& orthtree) : m_orthtree(orthtree) {}

  const Node* first() const {
    return deepest_first_child(m_orthtree, m_orthtree.root());
  }

  const Node* next(const Node* n) const {

    auto next = deepest_first_child(m_orthtree, next_sibling(m_orthtree, n));

    if (!next)
      next = n->parent();

    return next;
  }
};

/*!
  \ingroup PkgOrthtreeTraversal
  \brief A class used for performing a traversal on leaves only.

  All non-leave nodes are ignored.

  \cgalModels `OrthtreeTraversal`
 */
template <typename Tree>
struct Leaves_traversal {
private:

  using Node = typename Tree::Node;

  const Tree& m_orthtree;

public:

  Leaves_traversal(const Tree& orthtree) : m_orthtree(orthtree) {}

  const Node* first() const {
    return m_orthtree.deepest_first_child(&m_orthtree.root());
  }

  const Node* next(const Node* n) const {

    auto next = m_orthtree.deepest_first_child(m_orthtree.next_sibling(n));

    if (!next)
      next = m_orthtree.deepest_first_child(m_orthtree.next_sibling_up(n));

    return next;
  }
};

/*!
  \ingroup PkgOrthtreeTraversal
  \brief A class used for performing a traversal of a specific depth level.

  All trees at another depth are ignored. If the selected depth is
  higher than the maximum depth of the orthtree, no node will be traversed.

  \cgalModels `OrthtreeTraversal`
 */
template <typename Tree>
struct Level_traversal {
private:

  using Node = typename Tree::Node;

  const Tree& m_orthtree;
  const std::size_t m_depth;

public:

  /*!
    constructs a `depth`-level traversal.
  */
  Level_traversal(const Tree& orthtree, std::size_t depth) : m_orthtree(orthtree), m_depth(depth) {}

  template <typename Node>
  const Node* first() const {
    return m_orthtree.first_child_at_depth(m_orthtree.root(), m_depth);
  }

  template <typename Node>
  const Node* next(const Node* n) const {
    const Node* next = m_orthtree.next_sibling(n);

    if (!next) {
      const Node* up = n;
      do {
        up = m_orthtree.next_sibling_up(up);
        if (!up)
          return nullptr;

        next = m_orthtree.first_child_at_depth(up, m_depth);
      } while (!next);
    }

    return next;
  }
};

} // Orthtree
} // CGAL

#endif //CGAL_ORTHTREE_TRAVERSALS_H
