# zimg
# Courtesy of Stephen R. Savage
# https://github.com/sekrit-twc/zimg
# Until it is shaped into well organized library

INCLUDEPATH += $$PWD/../src/image/zimg

HEADERS += ../src/image/zimg/API/zimg.h
HEADERS += ../src/image/zimg/API/zimg++.hpp
HEADERS += ../src/image/zimg/Colorspace/colorspace.h
HEADERS += ../src/image/zimg/Colorspace/colorspace_param.h
HEADERS += ../src/image/zimg/Colorspace/graph.h
HEADERS += ../src/image/zimg/Colorspace/matrix3.h
HEADERS += ../src/image/zimg/Colorspace/operation.h
HEADERS += ../src/image/zimg/Colorspace/operation_impl.h
HEADERS += ../src/image/zimg/Colorspace/operation_impl_x86.h
HEADERS += ../src/image/zimg/Common/align.h
HEADERS += ../src/image/zimg/Common/cpuinfo.h
HEADERS += ../src/image/zimg/Common/except.h
HEADERS += ../src/image/zimg/Common/matrix.h
HEADERS += ../src/image/zimg/Common/osdep.h
HEADERS += ../src/image/zimg/Common/pixel.h
HEADERS += ../src/image/zimg/Common/plane.h
HEADERS += ../src/image/zimg/Depth/depth.h
HEADERS += ../src/image/zimg/Depth/depth_convert.h
HEADERS += ../src/image/zimg/Depth/depth_convert_x86.h
HEADERS += ../src/image/zimg/Depth/dither.h
HEADERS += ../src/image/zimg/Depth/dither_impl.h
HEADERS += ../src/image/zimg/Depth/dither_impl_x86.h
HEADERS += ../src/image/zimg/Depth/error_diffusion.h
HEADERS += ../src/image/zimg/Depth/quantize.h
HEADERS += ../src/image/zimg/Depth/quantize_avx2.h
HEADERS += ../src/image/zimg/Depth/quantize_sse2.h
HEADERS += ../src/image/zimg/Resize/filter.h
HEADERS += ../src/image/zimg/Resize/resize.h
HEADERS += ../src/image/zimg/Resize/resize_impl.h
HEADERS += ../src/image/zimg/Resize/resize_impl_x86.h

SOURCES += ../src/image/zimg/API/zimg.cpp
SOURCES += ../src/image/zimg/Colorspace/colorspace.cpp
SOURCES += ../src/image/zimg/Colorspace/colorspace_param.cpp
SOURCES += ../src/image/zimg/Colorspace/graph.cpp
SOURCES += ../src/image/zimg/Colorspace/matrix3.cpp
SOURCES += ../src/image/zimg/Colorspace/operation.cpp
SOURCES += ../src/image/zimg/Colorspace/operation_impl.cpp
SOURCES += ../src/image/zimg/Colorspace/operation_impl_avx2.cpp
SOURCES += ../src/image/zimg/Colorspace/operation_impl_sse2.cpp
SOURCES += ../src/image/zimg/Colorspace/operation_impl_x86.cpp
SOURCES += ../src/image/zimg/Depth/depth.cpp
SOURCES += ../src/image/zimg/Depth/depth_convert.cpp
SOURCES += ../src/image/zimg/Depth/depth_convert_avx2.cpp
SOURCES += ../src/image/zimg/Depth/depth_convert_sse2.cpp
SOURCES += ../src/image/zimg/Depth/depth_convert_x86.cpp
SOURCES += ../src/image/zimg/Depth/dither.cpp
SOURCES += ../src/image/zimg/Depth/dither_impl.cpp
SOURCES += ../src/image/zimg/Depth/dither_impl_avx2.cpp
SOURCES += ../src/image/zimg/Depth/dither_impl_sse2.cpp
SOURCES += ../src/image/zimg/Depth/dither_impl_x86.cpp
SOURCES += ../src/image/zimg/Depth/error_diffusion.cpp
SOURCES += ../src/image/zimg/Resize/filter.cpp
SOURCES += ../src/image/zimg/Resize/resize.cpp
SOURCES += ../src/image/zimg/Resize/resize_impl.cpp
SOURCES += ../src/image/zimg/Resize/resize_impl_avx2.cpp
SOURCES += ../src/image/zimg/Resize/resize_impl_sse2.cpp
SOURCES += ../src/image/zimg/Resize/resize_impl_x86.cpp
