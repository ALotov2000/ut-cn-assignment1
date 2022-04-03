CXX :=g++ -std=c++17
CPPFLAGS := -Wall -Werror
LFLAGS := -l jsoncpp -l pthread
SDIR := src
IDIR := include
ODIR := out
output := main.out

headerFiles := $(wildcard $(IDIR)/*.h)
sourceFiles := $(wildcard $(SDIR)/*.cpp)
objectFiles := $(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o, $(sourceFiles))


all: $(output)

$(output): $(objectFiles) $(headerFiles)
	$(CXX) $(CPPFLAGS) -o $@ $(objectFiles) $(LFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(headerFiles)
	$(CXX) $(CPPFLAGS) -o $@ -c $< $(LFLAGS)

clean:
	rm -f $(output) $(wildcard $(ODIR)/*.o)

run:
	@./main.out config.json