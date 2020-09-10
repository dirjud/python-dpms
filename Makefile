.PHONY: install test

all: install test

install:
	python setup.py install

test:
	python -m unittest tests/*.py
