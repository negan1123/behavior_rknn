targetout = behavior

CPP = aarch64-linux-gnu-g++

LIBS = -lrknn_api -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_core -lrockchip_mpp -lpthread -lavformat -lavdevice -lavfilter -lavcodec -lavutil -lswresample -lswscale

INCLUDES_PATH = -I../include -I/usr/local/include/opencv4

LIBS_PATH = -L../lib

DEFINED = -DRKNN_HAVE_MEAN_DEAL -DASIO_STANDALONE

source = $(wildcard *.cpp)

objects := $(source:%.cpp=%.o)

targetout: $(objects)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	$(CPP) $(INCLUDES_PATH) $(LIBS_PATH) $(DEFINED) $(objects) -o $(targetout) $(LIBS)

$(objects): %.o: %.cpp
	$(CPP) -c $(INCLUDES_PATH) $(DEFINED) $< -o $@

clean:
	rm -f *.o
	rm -f behavior
