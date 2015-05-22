#ifndef PERSISTQFLAGS_H
#define PERSISTQFLAGS_H

//#include <boost/serialization/serialization.hpp>
//#include <boost/serialization/split_free.hpp>
//#include <boost/serialization/version.hpp>
//#include <boost/serialization/nvp.hpp>
//#include <boost/serialization/utility.hpp>


//class AA {
//public:
//    AA(long a =0): _a(a) {}
//    long _a;
//};


//namespace boost {
//namespace serialization {

//template <class Archive>
//void save(Archive & ar, const AA & t, const unsigned int file_version)
//{
//    Q_UNUSED(file_version);

//    long i(t._a);
//    ar << boost::serialization::make_nvp("i", i);
//}

//template <class Archive, typename T>
//void load(Archive & ar, AA & t, const unsigned int file_version)
//{
//    Q_UNUSED(file_version);

//    long i(0);
//    ar >> boost::serialization::make_nvp("i", i);
//    t = AA(i);
//}

//} // namespace serialization
//} // namespace boost

//BOOST_SERIALIZATION_SPLIT_FREE(AA)



#endif // PERSISTQFLAGS_H
