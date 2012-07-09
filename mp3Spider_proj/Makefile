# Location of jar file
JARDIR = /usr/lib

# Location of documents
DOCDIR = /usr/share/doc/mp3Spider

# Compiler information
JCC = javac
JDOC = javadoc

SRC = src/org/gnu/mp3Spider/
JCIF = src/org/gnu/jcifs/
DST = blib/
DOC = $(DST)doc/

all:		$(DST)mp3Spider.jar \
			$(DOC)

install:	all
			cp $(DST)mp3Spider.jar $(JARDIR)
			cp -R $(DOC)* $(DOCDIR)/

$(DST)mp3Spider.jar: 	$(DST)
			@echo "Making jar"
			@jar -cvf $(DST)mp3Spider.jar -C $(DST) org 

$(DOC): 	$(SRC)mp3Spider.java \
			$(SRC)SmbLs.java \
			$(SRC)ShareVu.java
			@echo "Making Documents"
			@mkdir -p $(DOC)
			@touch $(DOC)
			@$(JDOC) -package -windowtitle "mp3Spider - Search files on an SMB network" -d $(DOC) -sourcepath src org.gnu.mp3Spider org.gnu.jcifs org.gnu.jcifs.util

$(DST):		$(SRC)mp3Spider.java \
			$(SRC)SmbLs.java \
			$(SRC)ShareVu.java
			@echo "Making classes"
			@mkdir -p $(DST)
			@touch $(DST)
			$(JCC) -d $(DST) -sourcepath src $(SRC)mp3Spider.java $(JCIF)*/*.java

clean:	
		rm -rf $(DST) $(DOC)
