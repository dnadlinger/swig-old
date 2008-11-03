/*
  a re-implementation of the compilercache scripts in C

  The idea is based on the shell-script compilercache by Erik Thiele <erikyyy@erikyyy.de>

   Copyright (C) Andrew Tridgell 2002
   Copyright (C) Martin Pool 2003
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "ccache.h"

/* the base cache directory */
char *cache_dir = NULL;

/* the directory for temporary files */
static char *temp_dir = NULL;

/* the debug logfile name, if set */
char *cache_logfile = NULL;

/* the argument list after processing */
static ARGS *stripped_args;

/* the original argument list */
static ARGS *orig_args;

/* the output filename being compiled to */
static char *output_file;

/* the source file */
static char *input_file;

/* the name of the file containing the cached object code */
static char *hashname;

/* the extension of the file after pre-processing */
static const char *i_extension;

/* the name of the temporary pre-processor file */
static char *i_tmpfile;

/* are we compiling a .i or .ii file directly? */
static int direct_i_file;

/* the name of the cpp stderr file */
static char *cpp_stderr;

/* the name of the statistics file */
char *stats_file = NULL;

/* can we safely use the unification hashing backend? */
static int enable_unify;

/* a list of supported file extensions, and the equivalent
   extension for code that has been through the pre-processor
*/
static struct {
	char *extension;
	char *i_extension;
} extensions[] = {
	{"c", "i"},
	{"C", "ii"},
	{"m", "mi"},
	{"cc", "ii"},
	{"CC", "ii"},
	{"cpp", "ii"},
	{"CPP", "ii"},
	{"cxx", "ii"},
	{"CXX", "ii"},
	{"c++", "ii"},
	{"C++", "ii"},
	{"i", "i"},
	{"ii", "ii"},
	{NULL, NULL}};

/*
  something went badly wrong - just execute the real compiler
*/
static void failed(void)
{
	char *e;

	/* delete intermediate pre-processor file if needed */
	if (i_tmpfile) {
		if (!direct_i_file) {
			unlink(i_tmpfile);
		}
		free(i_tmpfile);
		i_tmpfile = NULL;
	}

	/* delete the cpp stderr file if necessary */
	if (cpp_stderr) {
		unlink(cpp_stderr);
		free(cpp_stderr);
		cpp_stderr = NULL;
	}

	/* strip any local args */
	args_strip(orig_args, "--ccache-");

	if ((e=getenv("CCACHE_PREFIX"))) {
		char *p = find_executable(e, MYNAME);
		if (!p) {
			perror(e);
			exit(1);
		}
		args_add_prefix(orig_args, p);
	}

	execv(orig_args->argv[0], orig_args->argv);
	cc_log("execv returned (%s)!\n", strerror(errno));
	perror(orig_args->argv[0]);
	exit(1);
}


/* return a string to be used to distinguish temporary files 
   this also tries to cope with NFS by adding the local hostname 
*/
static const char *tmp_string(void)
{
	static char *ret;

	if (!ret) {
		char hostname[200];
		strcpy(hostname, "unknown");
#if HAVE_GETHOSTNAME
		gethostname(hostname, sizeof(hostname)-1);
#endif
		hostname[sizeof(hostname)-1] = 0;
		asprintf(&ret, "%s.%u", hostname, (unsigned)getpid());
	}

	return ret;
}


/* run the real compiler and put the result in cache */
static void to_cache(ARGS *args)
{
	char *path_stderr;
	char *tmp_stdout, *tmp_stderr, *tmp_hashname;
	struct stat st1, st2;
	int status;

	x_asprintf(&tmp_stdout, "%s/tmp.stdout.%s", temp_dir, tmp_string());
	x_asprintf(&tmp_stderr, "%s/tmp.stderr.%s", temp_dir, tmp_string());
	x_asprintf(&tmp_hashname, "%s/tmp.hash.%s.o", temp_dir, tmp_string());

	args_add(args, "-o");
	args_add(args, tmp_hashname);

	/* Turn off DEPENDENCIES_OUTPUT when running cc1, because
	 * otherwise it will emit a line like
	 *
	 *  tmp.stdout.vexed.732.o: /home/mbp/.ccache/tmp.stdout.vexed.732.i
	 *
	 * unsetenv() is on BSD and Linux but not portable. */
	putenv("DEPENDENCIES_OUTPUT");
	
	if (getenv("CCACHE_CPP2")) {
		args_add(args, input_file);
	} else {
		args_add(args, i_tmpfile);
	}
	status = execute(args->argv, tmp_stdout, tmp_stderr);
	args_pop(args, 3);

	if (stat(tmp_stdout, &st1) != 0 || st1.st_size != 0) {
		cc_log("compiler produced stdout for %s\n", output_file);
		stats_update(STATS_STDOUT);
		unlink(tmp_stdout);
		unlink(tmp_stderr);
		unlink(tmp_hashname);
		failed();
	}
	unlink(tmp_stdout);

	if (status != 0) {
		int fd;
		cc_log("compile of %s gave status = %d\n", output_file, status);
		stats_update(STATS_STATUS);

		fd = open(tmp_stderr, O_RDONLY | O_BINARY);
		if (fd != -1) {
			if (strcmp(output_file, "/dev/null") == 0 ||
			    rename(tmp_hashname, output_file) == 0 || errno == ENOENT) {
				if (cpp_stderr) {
					/* we might have some stderr from cpp */
					int fd2 = open(cpp_stderr, O_RDONLY | O_BINARY);
					if (fd2 != -1) {
						copy_fd(fd2, 2);
						close(fd2);
						unlink(cpp_stderr);
						cpp_stderr = NULL;
					}
				}

				/* we can use a quick method of
                                   getting the failed output */
				copy_fd(fd, 2);
				close(fd);
				unlink(tmp_stderr);
				if (i_tmpfile && !direct_i_file) {
					unlink(i_tmpfile);
				}
				exit(status);
			}
		}
		
		unlink(tmp_stderr);
		unlink(tmp_hashname);
		failed();
	}

	x_asprintf(&path_stderr, "%s.stderr", hashname);

	if (stat(tmp_stderr, &st1) != 0 ||
	    stat(tmp_hashname, &st2) != 0 ||
	    rename(tmp_hashname, hashname) != 0 ||
	    rename(tmp_stderr, path_stderr) != 0) {
		cc_log("failed to rename tmp files - %s\n", strerror(errno));
		stats_update(STATS_ERROR);
		failed();
	}

	cc_log("Placed %s into cache\n", output_file);
	stats_tocache(file_size(&st1) + file_size(&st2));

	free(tmp_hashname);
	free(tmp_stderr);
	free(tmp_stdout);
	free(path_stderr);
}

/* find the hash for a command. The hash includes all argument lists,
   plus the output from running the compiler with -E */
static void find_hash(ARGS *args)
{
	int i;
	char *path_stdout, *path_stderr;
	char *hash_dir;
	char *s;
	struct stat st;
	int status;
	int nlevels = 2;
	char *input_base;
	char *tmp;
	
	if ((s = getenv("CCACHE_NLEVELS"))) {
		nlevels = atoi(s);
		if (nlevels < 1) nlevels = 1;
		if (nlevels > 8) nlevels = 8;
	}

	hash_start();

	/* when we are doing the unifying tricks we need to include
           the input file name in the hash to get the warnings right */
	if (enable_unify) {
		hash_string(input_file);
	}

	/* we have to hash the extension, as a .i file isn't treated the same
	   by the compiler as a .ii file */
	hash_string(i_extension);

	/* first the arguments */
	for (i=1;i<args->argc;i++) {
		/* some arguments don't contribute to the hash. The
		   theory is that these arguments will change the
		   output of -E if they are going to have any effect
		   at all, or they only affect linking */
		if (i < args->argc-1) {
			if (strcmp(args->argv[i], "-I") == 0 ||
			    strcmp(args->argv[i], "-include") == 0 ||
			    strcmp(args->argv[i], "-L") == 0 ||
			    strcmp(args->argv[i], "-D") == 0 ||
			    strcmp(args->argv[i], "-idirafter") == 0 ||
			    strcmp(args->argv[i], "-isystem") == 0) {
				i++;
				continue;
			}
		}
		if (strncmp(args->argv[i], "-I", 2) == 0 ||
		    strncmp(args->argv[i], "-L", 2) == 0 ||
		    strncmp(args->argv[i], "-D", 2) == 0 ||
		    strncmp(args->argv[i], "-idirafter", 10) == 0 ||
		    strncmp(args->argv[i], "-isystem", 8) == 0) {
			continue;
		}

		if (strncmp(args->argv[i], "--specs=", 8) == 0 &&
		    stat(args->argv[i]+8, &st) == 0) {
			/* if given a explicit specs file, then hash that file, but
			   don't include the path to it in the hash */
			hash_file(args->argv[i]+8);
			continue;
		}

		/* all other arguments are included in the hash */
		hash_string(args->argv[i]);
	}

	/* the compiler driver size and date. This is a simple minded way
	   to try and detect compiler upgrades. It is not 100% reliable */
	if (stat(args->argv[0], &st) != 0) {
		cc_log("Couldn't stat the compiler!? (argv[0]='%s')\n", args->argv[0]);
		stats_update(STATS_COMPILER);
		failed();
	}

	/* also include the hash of the compiler name - as some compilers
	   use hard links and behave differently depending on the real name */
	if (st.st_nlink > 1) {
		hash_string(str_basename(args->argv[0]));
	}

	hash_int(st.st_size);
	hash_int(st.st_mtime);

	/* possibly hash the current working directory */
	if (getenv("CCACHE_HASHDIR")) {
		char *cwd = gnu_getcwd();
		if (cwd) {
			hash_string(cwd);
			free(cwd);
		}
	}

	/* ~/hello.c -> tmp.hello.123.i 
	   limit the basename to 10
	   characters in order to cope with filesystem with small
	   maximum filename length limits */
	input_base = str_basename(input_file);
	tmp = strchr(input_base, '.');
	if (tmp != NULL) {
		*tmp = 0;
	}
	if (strlen(input_base) > 10) {
		input_base[10] = 0;
	}

	/* now the run */
	x_asprintf(&path_stdout, "%s/%s.tmp.%s.%s", temp_dir,
		   input_base, tmp_string(), 
		   i_extension);
	x_asprintf(&path_stderr, "%s/tmp.cpp_stderr.%s", temp_dir, tmp_string());

	if (!direct_i_file) {
		/* run cpp on the input file to obtain the .i */
		args_add(args, "-E");
		args_add(args, input_file);
		status = execute(args->argv, path_stdout, path_stderr);
		args_pop(args, 2);
	} else {
		/* we are compiling a .i or .ii file - that means we
		   can skip the cpp stage and directly form the
		   correct i_tmpfile */
		path_stdout = input_file;
		if (create_empty_file(path_stderr) != 0) {
			stats_update(STATS_ERROR);
			cc_log("failed to create empty stderr file\n");
			failed();
		}
		status = 0;
	}

	if (status != 0) {
		if (!direct_i_file) {
			unlink(path_stdout);
		}
		unlink(path_stderr);
		cc_log("the preprocessor gave %d\n", status);
		stats_update(STATS_PREPROCESSOR);
		failed();
	}

	/* if the compilation is with -g then we have to include the whole of the
	   preprocessor output, which means we are sensitive to line number
	   information. Otherwise we can discard line number info, which makes
	   us less sensitive to reformatting changes 

	   Note! I have now disabled the unification code by default
	   as it gives the wrong line numbers for warnings. Pity.
	*/
	if (!enable_unify) {
		hash_file(path_stdout);
	} else {
		if (unify_hash(path_stdout) != 0) {
			stats_update(STATS_ERROR);
			failed();
		}
	}
	hash_file(path_stderr);

	i_tmpfile = path_stdout;

	if (!getenv("CCACHE_CPP2")) {
		/* if we are using the CPP trick then we need to remember this stderr
		   data and output it just before the main stderr from the compiler
		   pass */
		cpp_stderr = path_stderr;
	} else {	
		unlink(path_stderr);
		free(path_stderr);
	}

	/* we use a N level subdir for the cache path to reduce the impact
	   on filesystems which are slow for large directories
	*/
	s = hash_result();
	x_asprintf(&hash_dir, "%s/%c", cache_dir, s[0]);
	x_asprintf(&stats_file, "%s/stats", hash_dir);
	for (i=1; i<nlevels; i++) {
		char *p;
		if (create_dir(hash_dir) != 0) {
			cc_log("failed to create %s\n", hash_dir);
			failed();
		}
		x_asprintf(&p, "%s/%c", hash_dir, s[i]);
		free(hash_dir);
		hash_dir = p;
	}
	if (create_dir(hash_dir) != 0) {
		cc_log("failed to create %s\n", hash_dir);
		failed();
	}
	x_asprintf(&hashname, "%s/%s", hash_dir, s+nlevels);
	free(hash_dir);
}


/* 
   try to return the compile result from cache. If we can return from
   cache then this function exits with the correct status code,
   otherwise it returns */
static void from_cache(int first)
{
	int fd_stderr, fd_cpp_stderr;
	char *stderr_file;
	int ret;
	struct stat st;

	x_asprintf(&stderr_file, "%s.stderr", hashname);
	fd_stderr = open(stderr_file, O_RDONLY | O_BINARY);
	if (fd_stderr == -1) {
		/* it isn't in cache ... */
		free(stderr_file);
		return;
	}

	/* make sure the output is there too */
	if (stat(hashname, &st) != 0) {
		close(fd_stderr);
		unlink(stderr_file);
		free(stderr_file);
		return;
	}

	/* the user might be disabling cache hits */
	if (first && getenv("CCACHE_RECACHE")) {
		close(fd_stderr);
		unlink(stderr_file);
		free(stderr_file);
		return;
	}

	utime(stderr_file, NULL);

	if (strcmp(output_file, "/dev/null") == 0) {
		ret = 0;
	} else {
		unlink(output_file);
		if (getenv("CCACHE_HARDLINK")) {
			ret = link(hashname, output_file);
		} else {
			ret = copy_file(hashname, output_file);
		}
	}

	/* the hash file might have been deleted by some external process */
	if (ret == -1 && errno == ENOENT) {
		cc_log("hashfile missing for %s\n", output_file);
		stats_update(STATS_MISSING);
		close(fd_stderr);
		unlink(stderr_file);
		return;
	}
	free(stderr_file);

	if (ret == -1) {
		ret = copy_file(hashname, output_file);
		if (ret == -1) {
			cc_log("failed to copy %s -> %s (%s)\n", 
			       hashname, output_file, strerror(errno));
			stats_update(STATS_ERROR);
			failed();
		}
	}
	if (ret == 0) {
		/* update the mtime on the file so that make doesn't get confused */
		utime(output_file, NULL);
	}

	/* get rid of the intermediate preprocessor file */
	if (i_tmpfile) {
		if (!direct_i_file) {
			unlink(i_tmpfile);
		}
		free(i_tmpfile);
		i_tmpfile = NULL;
	}

	/* send the cpp stderr, if applicable */
	fd_cpp_stderr = open(cpp_stderr, O_RDONLY | O_BINARY);
	if (fd_cpp_stderr != -1) {
		copy_fd(fd_cpp_stderr, 2);
		close(fd_cpp_stderr);
		unlink(cpp_stderr);
		free(cpp_stderr);
		cpp_stderr = NULL;
	}

	/* send the stderr */
	copy_fd(fd_stderr, 2);
	close(fd_stderr);

	/* and exit with the right status code */
	if (first) {
		cc_log("got cached result for %s\n", output_file);
		stats_update(STATS_CACHED);
	}

	exit(0);
}

/* find the real compiler. We just search the PATH to find a executable of the 
   same name that isn't a link to ourselves */
static void find_compiler(int argc, char **argv)
{
	char *base;
	char *path;

	orig_args = args_init(argc, argv);

	base = str_basename(argv[0]);

	/* we might be being invoked like "ccache gcc -c foo.c" */
	if (strcmp(base, MYNAME) == 0) {
		args_remove_first(orig_args);
		free(base);
		if (strchr(argv[1],'/')) {
			/* a full path was given */
			return;
		}
		base = str_basename(argv[1]);
	}

	/* support user override of the compiler */
	if ((path=getenv("CCACHE_CC"))) {
		base = strdup(path);
	}

	orig_args->argv[0] = find_executable(base, MYNAME);

	/* can't find the compiler! */
	if (!orig_args->argv[0]) {
		stats_update(STATS_COMPILER);
		perror(base);
		exit(1);
	}
}


/* check a filename for C/C++ extension. Return the pre-processor
   extension */
static const char *check_extension(const char *fname, int *direct_i)
{
	int i;
	const char *p;

	if (direct_i) {
		*direct_i = 0;
	}

	p = strrchr(fname, '.');
	if (!p) return NULL;
	p++;
	for (i=0; extensions[i].extension; i++) {
		if (strcmp(p, extensions[i].extension) == 0) {
			if (direct_i && strcmp(p, extensions[i].i_extension) == 0) {
				*direct_i = 1;
			}
			p = getenv("CCACHE_EXTENSION");
			if (p) return p;
			return extensions[i].i_extension;
		}
	}
	return NULL;
}


/* 
   process the compiler options to form the correct set of options 
   for obtaining the preprocessor output
*/
static void process_args(int argc, char **argv)
{
	int i;
	int found_c_opt = 0;
	int found_S_opt = 0;
	struct stat st;
	char *e;

	stripped_args = args_init(0, NULL);

	args_add(stripped_args, argv[0]);

	for (i=1; i<argc; i++) {
		/* some options will never work ... */
		if (strcmp(argv[i], "-E") == 0) {
			failed();
		}

		/* these are too hard */
		if (strcmp(argv[i], "-fbranch-probabilities")==0 ||
		    strcmp(argv[i], "-M") == 0 ||
		    strcmp(argv[i], "-MM") == 0 ||
		    strcmp(argv[i], "-x") == 0) {
			cc_log("argument %s is unsupported\n", argv[i]);
			stats_update(STATS_UNSUPPORTED);
			failed();
			continue;
		}

		/* we must have -c */
		if (strcmp(argv[i], "-c") == 0) {
			args_add(stripped_args, argv[i]);
			found_c_opt = 1;
			continue;
		}

		/* -S changes the default extension */
		if (strcmp(argv[i], "-S") == 0) {
			args_add(stripped_args, argv[i]);
			found_S_opt = 1;
			continue;
		}
		
		/* we need to work out where the output was meant to go */
		if (strcmp(argv[i], "-o") == 0) {
			if (i == argc-1) {
				cc_log("missing argument to %s\n", argv[i]);
				stats_update(STATS_ARGS);
				failed();
			}
			output_file = argv[i+1];
			i++;
			continue;
		}
		
		/* alternate form of -o, with no space */
		if (strncmp(argv[i], "-o", 2) == 0) {
			output_file = &argv[i][2];
			continue;
		}

		/* debugging is handled specially, so that we know if we
		   can strip line number info 
		*/
		if (strncmp(argv[i], "-g", 2) == 0) {
			args_add(stripped_args, argv[i]);
			if (strcmp(argv[i], "-g0") != 0) {
				enable_unify = 0;
			}
			continue;
		}

		/* The user knows best: just swallow the next arg */
		if (strcmp(argv[i], "--ccache-skip") == 0) {
			i++;
			if (i == argc) {
				failed();
			}
			args_add(stripped_args, argv[i]);
			continue;
		}

		/* options that take an argument */
		{
			const char *opts[] = {"-I", "-include", "-imacros", "-iprefix",
					      "-iwithprefix", "-iwithprefixbefore",
					      "-L", "-D", "-U", "-x", "-MF", 
					      "-MT", "-MQ", "-isystem", "-aux-info",
					      "--param", "-A", "-Xlinker", "-u",
					      "-idirafter", 
					      NULL};
			int j;
			for (j=0;opts[j];j++) {
				if (strcmp(argv[i], opts[j]) == 0) {
					if (i == argc-1) {
						cc_log("missing argument to %s\n", 
						       argv[i]);
						stats_update(STATS_ARGS);
						failed();
					}
						
					args_add(stripped_args, argv[i]);
					args_add(stripped_args, argv[i+1]);
					i++;
					break;
				}
			}
			if (opts[j]) continue;
		}

		/* other options */
		if (argv[i][0] == '-') {
			args_add(stripped_args, argv[i]);
			continue;
		}

		/* if an argument isn't a plain file then assume its
		   an option, not an input file. This allows us to
		   cope better with unusual compiler options */
		if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
			args_add(stripped_args, argv[i]);
			continue;			
		}

		if (input_file) {
			if (check_extension(argv[i], NULL)) {
				cc_log("multiple input files (%s and %s)\n",
				       input_file, argv[i]);
				stats_update(STATS_MULTIPLE);
			} else if (!found_c_opt) {
				cc_log("called for link with %s\n", argv[i]);
				if (strstr(argv[i], "conftest.")) {
					stats_update(STATS_CONFTEST);
				} else {
					stats_update(STATS_LINK);
				}
			} else {
				cc_log("non C/C++ file %s\n", argv[i]);
				stats_update(STATS_NOTC);
			}
			failed();
		}

		input_file = argv[i];
	}

	if (!input_file) {
		cc_log("No input file found\n");
		stats_update(STATS_NOINPUT);
		failed();
	}

	i_extension = check_extension(input_file, &direct_i_file);
	if (i_extension == NULL) {
		cc_log("Not a C/C++ file - %s\n", input_file);
		stats_update(STATS_NOTC);
		failed();
	}

	if (!found_c_opt) {
		cc_log("No -c option found for %s\n", input_file);
		/* I find that having a separate statistic for autoconf tests is useful,
		   as they are the dominant form of "called for link" in many cases */
		if (strstr(input_file, "conftest.")) {
			stats_update(STATS_CONFTEST);
		} else {
			stats_update(STATS_LINK);
		}
		failed();
	}


	/* don't try to second guess the compilers heuristics for stdout handling */
	if (output_file && strcmp(output_file, "-") == 0) {
		stats_update(STATS_OUTSTDOUT);
		failed();
	}

	if (!output_file) {
		char *p;
		output_file = x_strdup(input_file);
		if ((p = strrchr(output_file, '/'))) {
			output_file = p+1;
		}
		p = strrchr(output_file, '.');
		if (!p || !p[1]) {
			cc_log("badly formed output_file %s\n", output_file);
			stats_update(STATS_ARGS);
			failed();
		}
		p[1] = found_S_opt ? 's' : 'o';
		p[2] = 0;
	}

	/* cope with -o /dev/null */
	if (strcmp(output_file,"/dev/null") != 0 && stat(output_file, &st) == 0 && !S_ISREG(st.st_mode)) {
		cc_log("Not a regular file %s\n", output_file);
		stats_update(STATS_DEVICE);
		failed();
	}

	if ((e=getenv("CCACHE_PREFIX"))) {
		char *p = find_executable(e, MYNAME);
		if (!p) {
			perror(e);
			exit(1);
		}
		args_add_prefix(stripped_args, p);
	}
}

/* the main ccache driver function */
static void ccache(int argc, char *argv[])
{
	/* find the real compiler */
	find_compiler(argc, argv);

	/* use the real compiler if HOME is not set */
	if (!cache_dir) {
		cc_log("Unable to determine home directory\n");
		cc_log("ccache is disabled\n");
		failed();
	}
	
	/* we might be disabled */
	if (getenv("CCACHE_DISABLE")) {
		cc_log("ccache is disabled\n");
		failed();
	}

	if (getenv("CCACHE_UNIFY")) {
		enable_unify = 1;
	}

	/* process argument list, returning a new set of arguments for pre-processing */
	process_args(orig_args->argc, orig_args->argv);

	/* run with -E to find the hash */
	find_hash(stripped_args);

	/* if we can return from cache at this point then do */
	from_cache(1);

	if (getenv("CCACHE_READONLY")) {
		cc_log("read-only set - doing real compile\n");
		failed();
	}
	
	/* run real compiler, sending output to cache */
	to_cache(stripped_args);

	/* return from cache */
	from_cache(0);

	/* oh oh! */
	cc_log("secondary from_cache failed!\n");
	stats_update(STATS_ERROR);
	failed();
}


static void usage(void)
{
	printf("ccache, a compiler cache. Version %s\n", CCACHE_VERSION);
	printf("Copyright Andrew Tridgell, 2002\n\n");
	
	printf("Usage:\n");
	printf("\tccache [options]\n");
	printf("\tccache compiler [compile options]\n");
	printf("\tcompiler [compile options]    (via symbolic link)\n");
	printf("\nOptions:\n");

	printf("-s                      show statistics summary\n");
	printf("-z                      zero statistics\n");
	printf("-c                      run a cache cleanup\n");
	printf("-C                      clear the cache completely\n");
	printf("-F <maxfiles>           set maximum files in cache\n");
	printf("-M <maxsize>            set maximum size of cache (use G, M or K)\n");
	printf("-h                      this help page\n");
	printf("-V                      print version number\n");
}

static void check_cache_dir(void)
{
	if (!cache_dir) {
		fatal("Unable to determine home directory");
	}
}

/* the main program when not doing a compile */
static int ccache_main(int argc, char *argv[])
{
	int c;
	size_t v;

	while ((c = getopt(argc, argv, "hszcCF:M:V")) != -1) {
		switch (c) {
		case 'V':
			printf("ccache version %s\n", CCACHE_VERSION);
			printf("Copyright Andrew Tridgell 2002\n");
			printf("Released under the GNU GPL v2 or later\n");
			exit(0);

		case 'h':
			usage();
			exit(0);
			
		case 's':
			check_cache_dir();
			stats_summary();
			break;

		case 'c':
			check_cache_dir();
			cleanup_all(cache_dir);
			printf("Cleaned cache\n");
			break;

		case 'C':
			check_cache_dir();
			wipe_all(cache_dir);
			printf("Cleared cache\n");
			break;

		case 'z':
			check_cache_dir();
			stats_zero();
			printf("Statistics cleared\n");
			break;

		case 'F':
			check_cache_dir();
			v = atoi(optarg);
			stats_set_limits(v, -1);
			printf("Set cache file limit to %u\n", (unsigned)v);
			break;

		case 'M':
			check_cache_dir();
			v = value_units(optarg);
			stats_set_limits(-1, v);
			printf("Set cache size limit to %uk\n", (unsigned)v);
			break;

		default:
			usage();
			exit(1);
		}
	}

	return 0;
}


/* Make a copy of stderr that will not be cached, so things like
   distcc can send networking errors to it. */
static void setup_uncached_err(void)
{
	char *buf;
	int uncached_fd;
	
	uncached_fd = dup(2);
	if (uncached_fd == -1) {
		cc_log("dup(2) failed\n");
		failed();
	}

	/* leak a pointer to the environment */
	x_asprintf(&buf, "UNCACHED_ERR_FD=%d", uncached_fd);

	if (putenv(buf) == -1) {
		cc_log("putenv failed\n");
		failed();
	}
}


int main(int argc, char *argv[])
{
	char *p;

	cache_dir = getenv("CCACHE_DIR");
	if (!cache_dir) {
		const char *home_directory = get_home_directory();
		if (home_directory) {
			x_asprintf(&cache_dir, "%s/.ccache", home_directory);
		}
	}

	temp_dir = getenv("CCACHE_TEMPDIR");
	if (!temp_dir) {
		temp_dir = cache_dir;
	}

	cache_logfile = getenv("CCACHE_LOGFILE");

	setup_uncached_err();
	

	/* the user might have set CCACHE_UMASK */
	p = getenv("CCACHE_UMASK");
	if (p) {
		mode_t mask;
		errno = 0;
		mask = strtol(p, NULL, 8);
		if (errno == 0) {
			umask(mask);
		}
	}


	/* check if we are being invoked as "ccache" */
	if (strlen(argv[0]) >= strlen(MYNAME) &&
	    strcmp(argv[0] + strlen(argv[0]) - strlen(MYNAME), MYNAME) == 0) {
		if (argc < 2) {
			usage();
			exit(1);
		}
		/* if the first argument isn't an option, then assume we are
		   being passed a compiler name and options */
		if (argv[1][0] == '-') {
			return ccache_main(argc, argv);
		}
	}

	/* make sure the cache dir exists */
	if (cache_dir && (create_dir(cache_dir) != 0)) {
		fprintf(stderr,"ccache: failed to create %s (%s)\n", 
			cache_dir, strerror(errno));
		exit(1);
	}

	ccache(argc, argv);
	return 1;
}
