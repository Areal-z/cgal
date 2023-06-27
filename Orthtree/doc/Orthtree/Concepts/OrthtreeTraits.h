/*!
  \ingroup PkgOrthtreeConcepts
  \cgalConcept

  The concept `OrthtreeTraits` defines the requirements for the
  template parameter of the `CGAL::Orthtree` class.

  \cgalHasModel `CGAL::Orthtree_traits_2<GeomTraits>`
  \cgalHasModel `CGAL::Orthtree_traits_3<GeomTraits>`
  \cgalHasModel `CGAL::Orthtree_traits_d<GeomTraits,Dimension>`
*/
class OrthtreeTraits
{
public:

  /// \name Types
  /// @{

  typedef unspecified_type Dimension; ///< Dimension type (see `CGAL::Dimension_tag`).
  typedef unspecified_type Bbox_d; ///< Bounding box type.
  typedef unspecified_type FT; ///< The number type of the %Cartesian coordinates of types `Point_d`
  typedef unspecified_type Point_d; ///< Point type.
  typedef unspecified_type Sphere_d; ///< The sphere type for neighbor queries.

  /*!
    A random access iterator type to enumerate the
    %Cartesian coordinates of a point.
  */
  typedef unspecified_type Cartesian_const_iterator_d;
  typedef std::array<FT, Dimension::value> Array; ///< Array used for easy point constructions.


  /*!
   * \brief List-like or iterable type contained by each node
   *
   * Should provide begin() and end() iterators which span all the items contained by a node.
   * Must also be default-constructible, because node data is allocated ahead of time.
   * Many split predicates also expect a `Node_data::size()` method.
   * For example, this could be a `boost::range` of point indices, or an `std::vector` containing primitives.
   *
   * todo: For an empty tree, this should contain something like `std::array<Point_d, 0>`.
   * This way, nearest_neighbors still compiles, and simply returns nothing because all nodes are empty.
   * Eventually, nearest_neighbors will be removed and/or moved, and then this won't have to behave like a list.
   * Once that's done, `boost::none_t` will work for an empty tree.
   */
  typedef unspecified_type Node_data;

  /*!
   * \brief An element of the `Node_data` list-like type.
   *
   * Must be constructible from the type produced by dereferencing a `Node_data` iterator.
   * Typically the same as that type, but can also be an `std::reference_wrapper<>` if the type is not copyable.
   *
   * todo: This is only used as part of the return type for `nearest_neighbors()`.
   * Because `nearest_neighbors()` may be ill defined for empty node types,
   * this can be omitted in the final version of Orthtree_traits.
   */
  typedef unspecified_type Node_data_element;

  typedef unspecified_type Adjacency; ///< Specify the adjacency directions

  /*!
    Functor with an operator to construct a `Point_d` from an `Array` object.
  */
  typedef unspecified_type Construct_point_d_from_array;

  /*!
    Functor with an operator to construct a `Bbox_d` from two `Array` objects (coordinates of minimum and maximum points).
  */
  typedef unspecified_type Construct_bbox_d;

  /// @}

  /// \name Operations
  /// @{

  /*!
    Function used to construct an object of type `Construct_point_d_from_array`.
  */
  Construct_point_d_from_array construct_point_d_from_array_object() const;

  /*!
    Function used to construct an object of type `Construct_bbox_d`.
  */
  Construct_bbox_d construct_bbox_d_object() const;

  /*!
   * \brief Produces a bounding box which encloses the contents of the tree
   *
   * The bounding box must enclose all elements contained by the tree.
   * It may be tight-fitting, the orthtree constructor produces a bounding cube surrounding this region.
   * For traits which assign no data to each node, this can be defined to return a fixed region.
   *
   * @return std::pair<min, max>, where min and max represent cartesian corners which define a bounding box
   */
  std::pair<Array, Array> root_node_bbox() const;

  /*!
   * \brief Initializes the contained elements for the root node.
   *
   * Typically produces a `Node_data` which contains all the elements in the tree.
   *
   * @return The `Node_data` instance to be contained by the root node
   */
  Node_data root_node_contents() const;

  /*!
   * \brief Distributes the `Node_data` contents of a node to its immediate children.
   *
   * Invoked after a node is split.
   * Adds the contents of the node n to each of its children.
   * May rearrange or modify n's `Node_data`, but generally expected not to reset n.
   * After distributing n's contents, n should still have an list of elements it encloses.
   * Each of n's children should have an accurate list of the subset of elements within n they enclose.
   *
   * For an empty tree, this can be a null-op.
   *
   * @tparam Node_index The index type used by an orthtree implementation
   * @tparam Tree An Orthree implementation
   *
   * @param n The index of the node who's contents must be distributed.
   * @param tree The Orthtree which n belongs to
   * @param center The coordinate center of the node n, which its contents should be split around.
   */
  template <typename Node_index, typename Tree>
  void distribute_node_contents(Node_index n, Tree& tree, const Point_d& center);

  /// @}
};
