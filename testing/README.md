# Welcome to Testing! #

## Important ##
1. DO NOT f*ck with the organization of this directory. Please.
2. All testing can be run from the project root makefile, with the command `make test`.
3. If you need to clean up the testing directory, run `make testclean`.

## Troubleshooting ##
If things aren't running, ensure that all ".run" files are executable.
`chmod 777 file.run` should do the trick.
If that's not the problem, it's Eliott's fault and you should tell him.

## Organization ##
1. Project root makefile contains commands to build the test files.
2. "run_tests.sh" is a simple script that runs all the tests.
3. Any file ending in ".run" is actually a shell script to test various inputs to a testing binary.
4. Any file ending in ".test" is a make-generated binary executable, which is run by the corresponding ".run" file.

## To Do ##
Everything
