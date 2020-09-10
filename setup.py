import setuptools

module = setuptools.Extension("dpms", libraries=["Xext",], sources=["./src/pydpms.c"])

if __name__ == "__main__":
    setuptools.setup(
        name="dpms",
        version="1.0",
        author="Thiago Kenji Okada",
        author_email="thiagokokada@gmail.com",
        description="Python Bindings to DPMS X11 extension",
        ext_modules=[module],
    )
