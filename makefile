#Name of program
MAIN     = $@

ABS      = ~/source/enhance
BIN      = ~/bin
BUILD      = ~/source/enhance/build
RM       = /bin/rm -f
MV         = /bin/mv -f
CFLAGS   = -isystem /usr/local/include/opencv4/ -I /usr/lib/boost -Wno-deprecated-declarations -g -std=c++11 -rdynamic -pthread
CC       = /usr/bin/c++ $(CFLAGS)

           # OpenCV
LIBS =     -lopencv_gapi -lopencv_stitching -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired    \
           -lopencv_ccalib -lopencv_cvv -lopencv_dnn_objdetect -lopencv_dpm -lopencv_face          \
           -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash            \
           -lopencv_line_descriptor -lopencv_quality -lopencv_reg -lopencv_rgbd -lopencv_saliency   \
           -lopencv_sfm -lopencv_stereo -lopencv_structured_light -lopencv_superres                  \
           -lopencv_surface_matching -lopencv_tracking -lopencv_videostab -lopencv_xfeatures2d       \
           -lopencv_xobjdetect -lopencv_xphoto -lopencv_shape -lopencv_datasets -lopencv_plot      \
           -lopencv_text -lopencv_dnn -lopencv_highgui -lopencv_ml -lopencv_phase_unwrapping       \
           -lopencv_optflow -lopencv_ximgproc -lopencv_video -lopencv_videoio -lopencv_imgcodecs   \
           -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_photo   \
           -lopencv_imgproc -lopencv_core                                                            \
           -lavutil -lavcodec -lavformat -lavdevice -lavfilter -lswscale                            \
           -lboost_program_options -lncurses
                    
LFLAGS   = -Wl,-rpath,/usr/local/lib
LIBDIRS   = $(LFLAGS) -L/usr/local/lib/ -L/usr/lib/boost

#Output coloring
GREEN   = \033[1;32m
CYAN    = \033[36m
BLUE    = \033[1;34m
BRIGHT  = \033[1;37m
WHITE   = \033[0;m
MAGENTA = \033[35m
YELLOW  = \033[33m
RED     = \033[91m

#Source files
OBJS   = $(BUILD)/enhance.o               \
        $(BUILD)/processing.o             \
        $(BUILD)/compute.o                \
        $(BUILD)/iocustom.o
        
STARTRAILS = $(BUILD)/startrails.o       \
             $(OBJS)

TEST         = $(BUILD)/test.o             \
             $(OBJS)
             
STACKER      = $(BUILD)/stacker.o           \
             $(OBJS)

COADD        = $(BUILD)/coadd.o              \
             $(OBJS)

ADV_COADD  = $(BUILD)/advanced_coadd.o    \
             $(OBJS)

#Builds
all: 
   @printf "[                                               ]\n"
   @printf "[      $(RED)Error$(WHITE) - No build target specified.       ]\n"
   @printf "[                $(YELLOW)Exiting.$(WHITE)                       ]\n"
   @printf "[                                               ]\n"

$(BUILD)/%.o: %.c++
   @printf "[$(CYAN)Building$(WHITE)]   $(BRIGHT)$<$(WHITE) - $(MAGENTA)Object$(WHITE)\n"
   cd $(ABS); $(CC) -c -o $@ $<
   @printf "[$(GREEN) Built  $(WHITE)]   $(BRIGHT)$<$(WHITE) - $(MAGENTA)Object$(WHITE)\n"

$(MAIN).o: $(MAIN).c++
   @printf "[$(CYAN)Building$(WHITE)]   $(BRIGHT)$<$(WHITE) - $(MAGENTA)Object$(WHITE)\n"
   cd $(ABS); $(CC) -c $(MAIN).c++ -o $(MAIN).o
   @printf "[$(GREEN) Built  $(WHITE)]   $(BRIGHT)$<$(WHITE) - $(MAGENTA)Object$(WHITE)\n"

advanced_coadd: $(ADV_COADD)
   @printf "[$(CYAN)Linking $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"
   cd $(ABS); $(CC) $(ADV_COADD) $(LIBDIRS) -o $(BIN)/$@ $(LIBS)
   @printf "[$(GREEN)Linked  $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"

coadd: $(COADD)
   @printf "[$(CYAN)Linking $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"
   cd $(ABS); $(CC) $(COADD) $(LIBDIRS) -o $(BIN)/$@ $(LIBS)
   @printf "[$(GREEN)Linked  $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"

stacker: $(STACKER)
   @printf "[$(CYAN)Linking $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"
   cd $(ABS); $(CC) $(STACKER) $(LIBDIRS) -o $(BIN)/$@ $(LIBS)
   @printf "[$(GREEN)Linked  $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"

startrails: $(STARTRAILS)
   @printf "[$(CYAN)Linking $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"
   cd $(ABS); $(CC) $(STARTRAILS) $(LIBDIRS) -o $(BIN)/$@ $(LIBS)
   @printf "[$(GREEN)Linked  $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"

test: $(TEST)
   @printf "[$(CYAN)Linking $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"
   cd $(ABS); $(CC) $(TEST) $(LIBDIRS) -o ./$@ $(LIBS)
   @printf "[$(GREEN)Linked  $(WHITE)]   $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Binary$(WHITE)\n"   

clean:
   $(RM) *.core $(BUILD)/*.o *.d *.stackdump

#Disable command echoing, reenabled with make verbose=1
ifndef verbose
.SILENT:
endif
