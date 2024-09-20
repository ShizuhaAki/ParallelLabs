project('Parallel-MAC-Stage1', 'c', version: '1.0')

# Define sources
sources = [
  'main.c', 'queue.c', 'thpool.c', 'threaded_apply.c', 'threaded_map.c'
]

# Define the headers (if needed for installation or IDEs)
headers = [
  'queue.h', 'thpool.h', 'threaded_apply.h', 'threaded_map.h', 'def.h'
]

# Add compiler flags if necessary
cflags = ['-Wall', '-Wextra', '-pthread', '-fsanitize=undefined,address']

# Build the executable
executable('main', 
  sources, 
  install: true, 
  c_args: cflags, 
  link_args: ['-pthread', '-lm', '-fsanitize=undefined,address']
)
