# mostly copied from YCM's file
import os

DIR_OF_THIS_SCRIPT = os.path.abspath( os.path.dirname( __file__ ) )

flags = [
# general flags you would give to clang
'-Wall',
'-Wextra',
'-Wpedantic',
'-Wno-empty-translation-unit',
'-fexceptions',
'-iquote', DIR_OF_THIS_SCRIPT,
#'-x', 'c++',
#'-std=gnu++17',
]

include_dirs = [
# paths to be included using '-I'
# e.g. '/usr/include/X11/'
]

# ------------------------------------------------------

# if 'c++' in flags or 'cpp' in flags:
#     include_dirs.append('/usr/include/c++/10/')

flags += list(map(lambda x: '-I' + x, include_dirs))

DIR_OF_THIRD_PARTY = os.path.expanduser('~') + "/.vim/third_party"
SOURCE_EXTENSIONS = [ '.cpp', '.cxx', '.cc', '.c', '.m', '.mm' ]

def IsHeaderFile( filename ):
  extension = os.path.splitext( filename )[ 1 ]
  return extension in [ '.h', '.hxx', '.hpp', '.hh' ]

def FindCorrespondingSourceFile( filename ):
  if IsHeaderFile( filename ):
    basename = os.path.splitext( filename )[ 0 ]
    for extension in SOURCE_EXTENSIONS:
      replacement_file = basename + extension
      if os.path.exists( replacement_file ):
        return replacement_file
  return filename


# return al necessary compilation flags
def Settings( **kwargs ):
  if kwargs[ 'language' ] == 'cfamily':
    # If the file is a header, try to find the corresponding source file and
    # retrieve its flags from the compilation database if using one. This is
    # necessary since compilation databases don't have entries for header files.
    # In addition, use this source file as the translation unit. This makes it
    # possible to jump from a declaration in the header file to its definition
    # in the corresponding source file.
    filename = FindCorrespondingSourceFile( kwargs[ 'filename' ] )

    result = {
            'flags': flags,
            'override_filename': filename
            }
    if DIR_OF_THIS_SCRIPT != os.path.expanduser('~') + "/.vim":
        result = {**result, **{
            'include_paths_relative_to_dir': DIR_OF_THIS_SCRIPT
        }}

    return result

  return {}
