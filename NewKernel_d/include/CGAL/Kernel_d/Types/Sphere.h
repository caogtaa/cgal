#ifndef CGAL_KD_TYPE_SPHERE_H
#define CGAL_KD_TYPE_SPHERE_H
#include <CGAL/Kernel_d/store_kernel.h>
#include <boost/iterator/counting_iterator.hpp>
namespace CGAL {
template <class R_> class Sphere {
	typedef typename Get_type<R_, FT_tag>::type FT_;
	typedef typename Get_type<R_, Point_tag>::type	Point_;
	Point_ c_;
	FT_ r2_;

	public:
	Sphere(Point_ const&p, FT_ const&r2): c_(p), r2_(r2) {}
	// TODO: Add a piecewise constructor?

	Point_ const& center()const{return c_;}
	FT_ const& squared_radius()const{return r2_;}
};

namespace CartesianDKernelFunctors {
template <class R_> struct Construct_sphere : Store_kernel<R_> {
  CGAL_FUNCTOR_INIT_STORE(Construct_sphere)
  typedef typename Get_type<R_, Sphere_tag>::type	result_type;
  typedef typename Get_type<R_, Point_tag>::type	Point;
  typedef typename Get_type<R_, FT_tag>::type FT;
  result_type operator()(Point const&a, FT const&b)const{
    return result_type(a,b);
  }
  template <class Iter>
  result_type operator()(Iter f, Iter e)const{
    // 2*(x-y).c == x^2-y^2
    typedef typename R_::LA LA;
    typedef typename LA::Square_matrix Matrix;
    typedef typename LA::Vector Vec;
    typedef typename LA::Construct_vector CVec;
    typedef typename Get_type<R_, Point_tag>::type	Point;
    typename Get_functor<R_, Compute_point_cartesian_coordinate_tag>::type c(this->kernel());
    typename Get_functor<R_, Construct_ttag<Point_tag> >::type cp(this->kernel());
    typename Get_functor<R_, Point_dimension_tag>::type pd(this->kernel());
    typename Get_functor<R_, Squared_distance_to_origin_tag>::type sdo(this->kernel());
    typename Get_functor<R_, Squared_distance_tag>::type sd(this->kernel());

    Point const& p0=*f;
    int d = pd(p0);
    FT const& n0 = sdo(p0);
    Matrix m(d,d);
    Vec b = typename CVec::Dimension()(d);
    // Write the point coordinates in lines.
    int i;
    for(i=0; ++f!=e; ++i) {
      Point const& p=*f;
      for(int j=0;j<d;++j) {
	m(i,j)=2*(c(p,j)-c(p0,j));
	b[i] = sdo(p) - n0;
      }
    }
    CGAL_assertion (i == d);
    Vec res = typename CVec::Dimension()(d);;
    //std::cout << "Mat: " << m << "\n Vec: " << one << std::endl;
    LA::solve(res, CGAL_MOVE(m), CGAL_MOVE(b));
    //std::cout << "Sol: " << res << std::endl;
    Point center = cp(d,LA::vector_begin(res),LA::vector_end(res));
    FT const& r2 = sd (center, p0);
    return this->operator()(CGAL_MOVE(center), r2);
  }
};

template <class R_> struct Center_of_sphere : private Store_kernel<R_> {
  CGAL_FUNCTOR_INIT_STORE(Center_of_sphere)
  typedef typename Get_type<R_, Sphere_tag>::type	Sphere;
  typedef typename Get_type<R_, Point_tag>::type const&	result_type;
  result_type operator()(Sphere const&s)const{
    return s.center();
  }

  template<class Iter>
  result_type operator()(Iter b, Iter e)const{
    typename Get_functor<R_, Construct_ttag<Sphere_tag> >::type	cs(this->kernel());
    return operator()(cs(b,e)); // computes the radius needlessly
  }
};

template <class R_> struct Squared_radius {
  CGAL_FUNCTOR_INIT_IGNORE(Squared_radius)
  typedef typename Get_type<R_, Sphere_tag>::type	Sphere;
  typedef typename Get_type<R_, FT_tag>::type const&	result_type;
  // TODO: Is_exact?
  result_type operator()(Sphere const&s)const{
    return s.squared_radius();
  }
};

// FIXME: Move it to the generic functors, using the two above and conditional to the existence of sqrt(FT)
template<class R_> struct Point_of_sphere : private Store_kernel<R_> {
  CGAL_FUNCTOR_INIT_STORE(Point_of_sphere)
  typedef R_ R;
  typedef typename Get_type<R, FT_tag>::type FT;
  typedef typename Get_type<R, RT_tag>::type RT;
  typedef typename Get_type<R, Point_tag>::type Point;
  typedef typename Get_type<R, Sphere_tag>::type Sphere;
  typedef typename Get_functor<R, Construct_ttag<Point_tag> >::type CP;
  typedef typename Get_functor<R, Construct_ttag<Point_cartesian_const_iterator_tag> >::type CI;
  typedef typename Get_functor<R, Point_dimension_tag>::type PD;
  typedef Point result_type;
  typedef Sphere first_argument_type;
  typedef int second_argument_type;
  struct Trans : std::binary_function<FT,FT,int> {
    FT const& r_; int idx; bool sgn;
    Trans (int n, FT const& r, bool b) : r_(r), idx(n), sgn(b) {}
    FT operator()(FT const&x, int i)const{
      return (i == idx) ? sgn ? x + r_ : x - r_ : x;
    }
  };
  result_type operator()(Sphere const&s, int i)const{
    CI ci(this->kernel());
    PD pd(this->kernel());
    typedef boost::counting_iterator<int,std::random_access_iterator_tag> Count;
    Point const&c = s.center();
    int d=pd(c);
    bool last = (i == d);
    Trans t(last ? 0 : i, sqrt(s.radius()), !last);
    return CP(this->kernel())(make_transforming_pair_iterator(ci(c,Begin_tag()),Count(0),t),make_transforming_pair_iterator(ci(c,End_tag()),Count(d+1),t));
  }
};
}
CGAL_KD_DEFAULT_TYPE(Sphere_tag,(CGAL::Sphere<K>),(Point_tag),());
CGAL_KD_DEFAULT_FUNCTOR(Construct_ttag<Sphere_tag>,(CartesianDKernelFunctors::Construct_sphere<K>),(Sphere_tag,Point_tag),(Construct_ttag<Point_tag>,Compute_point_cartesian_coordinate_tag,Squared_distance_tag,Squared_distance_to_origin_tag,Point_dimension_tag));
CGAL_KD_DEFAULT_FUNCTOR(Center_of_sphere_tag,(CartesianDKernelFunctors::Center_of_sphere<K>),(Sphere_tag,Point_tag),(Construct_ttag<Sphere_tag>));
CGAL_KD_DEFAULT_FUNCTOR(Squared_radius_tag,(CartesianDKernelFunctors::Squared_radius<K>),(Sphere_tag),());
CGAL_KD_DEFAULT_FUNCTOR(Point_of_sphere_tag,(CartesianDKernelFunctors::Point_of_sphere<K>),(Sphere_tag,Point_tag),(Construct_ttag<Point_tag>, Construct_ttag<Point_cartesian_const_iterator_tag>));
} // namespace CGAL
#endif
