
#ifndef __LARCV_PRODUCTMAP_H__
#define __LARCV_PRODUCTMAP_H__

#include <string>
#include "DataFormatTypes.h"
namespace larcv {

  const std::string ProductName(ProductType_t type);

  template<class T> ProductType_t ProductType();

  class Image2D;
  template<> ProductType_t ProductType< larcv::Image2D  > ();
  class ROI;
  template<> ProductType_t ProductType< larcv::ROI      > ();
  class ChStatus;
  template<> ProductType_t ProductType< larcv::ChStatus > ();

}

#endif
