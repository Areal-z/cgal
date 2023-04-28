namespace CGAL {

/*!
\ingroup PkgDrawLinearCellComplex

opens a new window and draws `alcc`, a model of the `LinearCellComplex` concept. A call to this function is blocking, that is the program continues as soon as the user closes the window. This function requires `CGAL_Qt5`, and is only available if the macro `CGAL_USE_BASIC_VIEWER` is defined.
Linking with the cmake target `CGAL::CGAL_Basic_viewer` will link with `CGAL_Qt5` and add the definition `CGAL_USE_BASIC_VIEWER`.
\tparam LCC a model of the `LinearCellComplex` concept.
\param alcc the linear cell complex to draw.

*/
  template<class LCC, class DrawingFunctor>
  void draw(const LCC& alcc, const DrawingFunctor& afunctor);

/*!
\ingroup PkgDrawLinearCellComplex

*/
  template<class LCC, class BufferType, class DrawingFunctor>
  void add_in_graphic_storage(const LCC& alcc, CGAL::Graphic_storage<BufferType>& graphic_buffer,
                              const DrawingFunctor& afunctor);

} /* namespace CGAL */

