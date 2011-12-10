# Makes all the libraries needed for creating the complete RAG library

LIBRARIES = liblist.a librs.a libemd.a libpnmadd.a libRAG.a
TARGET = libFullRAG.a

all:
	# echo "Making list library"
	cd list; make
	
	# echo "Making rs library"
	cd rs; make

	# echo "Making EMD library"
	cd emd; make
	
	# echo "Making pnmadd library"
	cd pnmadd; make
	
	# echo "Making region library"
	cd region; make
	
	# echo "Making RAG library"
	cd RAG; make
	
	# echo "Making the FullRAG library"
	#cd lib; for libfile in ${LIBRARIES}; do ar x $$libfile; done
	#cd lib; ar cr ${TARGET} *.o; ranlib ${TARGET}; rm -f *.o
	
	# Create links for all the header files in the include directory
	ln list/*.h rs/*.h emd/*.h pnmadd/*.h region/*.h RAG/*.h include/

clean:
	cd list; make clean
	cd rs; make clean
	cd emd; make clean
	cd pnmadd; make clean
	cd region; make clean
	cd RAG; make clean
	cd lib; rm -f *
	cd include; rm -f *
	rm -f ${TARGET}

