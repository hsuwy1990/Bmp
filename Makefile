OUTDIR = .
APP = main
SRC = BMP.cpp main.cpp

INC = -I.
CFLAGS =

APP_OBJS = $(SRC:.cpp=.o)

all: $(APP)

$(APP): $(APP_OBJS)
	$(CXX) -o $(OUTDIR)/$@ $(APP_OBJS)

.cpp.o:
	$(CXX) -c $(CFLAGS) $(INC) $< -o $@

.PHONY: clean
clean:
	@-rm -f $(APP) $(APP_OBJS)
