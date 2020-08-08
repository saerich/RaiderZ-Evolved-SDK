#ifndef __AVG_DLX_H__
#define __AVG_DLX_H__

inline Point3 DPoint3toPoint3(const DPoint3& v) 
{	return Point3(v.x,v.y,v.z); 
}

template <class T>
class SimpleDataStore
{
public:
	SimpleDataStore(int isize) {data = new T [isize];size=isize;}
	~SimpleDataStore() {delete [] data;}
	T& operator[](int index) {assert ((unsigned int)index < size);return data[index];}
	T *data;
	int size;
};


#define ToTCBUI(a) (((a)+1.0f)*25.0f)  // HEY!! pinched from TCBINTRP.CPP, why not in a header or documented?
#define FromTCBUI(a) (((a)/25.0f)-1.0f)
#define ToEaseUI(a) ((a)*50.0f)
#define FromEaseUI(a) ((a)/50.0f)


#endif //__AVG_DLX_H__














