all:

.PHONY: doc docs doc-html doc-pdf
doc docs: doc-html

export VERSION:=$(shell cat ../VERSION)

doc-html:
	cd doc && doxygen 2>&1 >/dev/null | ./doxygen-err-filter.sh | ( ! grep . )

doc-pdf:
	$(MAKE) -C doc/latex
	mv doc/latex/refman.pdf doc/Awa.pdf
	rm -rf doc/latex

.PHONY: clean
clean:
	rm -rf doc/latex
	rm -rf doc/html
	rm -f doc/Awa.pdf
