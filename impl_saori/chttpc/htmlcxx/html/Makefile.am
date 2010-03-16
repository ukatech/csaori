lib_LTLIBRARIES = libhtmlcxx.la
libhtmlcxx_la_SOURCES = ParserSax.h ParserSax.tcc ParserSax.cc \
	ParserDom.h ParserDom.cc \
	Extensions.h Extensions.cc \
	CharsetConverter.h CharsetConverter.cc \
	Uri.h Uri.cc \
	Node.h Node.cc \
	utils.h utils.cc \
	ci_string.h \
	tree.h \
	wincstring.h\
	debug.h
libhtmlcxx_la_CXXFLAGS =
libhtmlcxx_la_LDFLAGS = -version-info 4:0:1

# bin_PROGRAMS = htmlcxx
# htmlcxx_LDADD = libhtmlcxx.la
# htmlcxx_SOURCES = htmlcxx.cc
						
noinst_PROGRAMS = tests uriTests
tests_SOURCES = tests.cc 
tests_LDADD = libhtmlcxx.la
uriTests_SOURCES = uriTests.cc
uriTests_LDADD = libhtmlcxx.la

TESTS = tests uriTests

includedir = $(prefix)/include/htmlcxx/html
include_HEADERS = ParserSax.h ParserSax.tcc ParserDom.h utils.h tree.h Node.h Uri.h CharsetConverter.h Extensions.h ci_string.h tld.h

EXTRA_DIST = gen_tld.pl tld.list
