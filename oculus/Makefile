#CXX = g++
CXX = nvcc
CFLAGS = `pkg-config --cflags opencv` -std=c++11 -O2 \
	-D_MWAITXINTRIN_H_INCLUDED \
	-D_FORCE_INLINES \
	-D__STRICT_ANSI__
LIBS = `pkg-config --libs opencv` -larmadillo -lpthread -lstdc++
OBJS = highgui.o imgproc.o xrift.o \
			 gcube.o gpu_util.o

all: $(OBJS) xrift

gcube.o: gcube.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

highgui.o: highgui.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

gpu_util.o: gpu_util.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

imgproc.o: imgproc.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

feature.o: feature.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

ovr.o: ovr.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

test.o: full_pipeline.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<

ovr: $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

xrift: $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

xrift.o: mvp.cu
	$(CXX) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) ovr
