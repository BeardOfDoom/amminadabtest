all:	clean validate_xmllint NLP_triplet_extraction.pdf
epub:	clean validate_xmllint NLP_triplet_extraction.epub

NLP_triplet_extraction.pdf: NLP_triplet_extraction.xml NLP_triplet_extraction.xls
	dblatex -T native NLP_triplet_extraction.xml -p NLP_triplet_extraction.xls

NLP_triplet_extraction.epub: NLP_triplet_extraction.xml
	dbtoepub -o NLP_triplet_extraction.epub NLP_triplet_extraction.xml


.PHONY: validate_jing
validate_jing:
	@if [ ! -e jing.jar ]; then \
		wget https://jing-trang.googlecode.com/files/jing-20091111.zip; \
		unzip jing-20091111.zip > /dev/null; \
		mv jing-20091111/bin/jing.jar jing.jar; \
		rm -rf jing-20091111*; \
	fi

	xmllint --xinclude NLP_triplet_extraction.xml --output output.xml
	java -jar jing.jar -c http://docbook.org/xml/5.1CR3/rng/docbookxi.rnc output.xml
	@rm -f output.xml


.PHONY: validate_xmllint
validate_xmllint:
	xmllint --xinclude NLP_triplet_extraction.xml --output output.xml
	xmllint --relaxng http://docbook.org/xml/5.1CR3/rng/docbookxi.rng output.xml --noout
	rm -f output.xml


.PHONY: clean
clean:
	rm -vf NLP_triplet_extraction.pdf NLP_triplet_extraction.epub