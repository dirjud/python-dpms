from distutils.core import setup, Extension

module = Extension('dpms',
                   libraries = ['Xext', ],
                   sources = ['pydpms.c'])

if __name__ == "__main__":
    setup (name = 'dpms',
           version = "1.0",
           author = "Lane Brooks",
           author_email = "dirjud@gmail.com",
           description = 'Python Bindings to DPMS X11 extension',
           ext_modules = [module])
