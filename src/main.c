/*
 * cment - an absurdly simple dependency tool for cmake
 * (c) 2016 Tim Lukins
 * See LICENCE.txt for rights and distribution.
 */

#include "ogdl.h"
#include "argparse.h"
#include <stdio.h>
#include <string.h>

char header[] ={
"###############################################################################\n"
"# AUTOGENERATED FILE\n"
"# Created by cment\n"
"# DO NOT EDIT DIRECTLY\n"
"# Best regenerated\n"
"###############################################################################\n"
"include(ExternalProject)\n"
"set(GLOBAL_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)\n"
"set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)\n"
"# Sets global output directory for single configuration (GCC)\n"
"set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})\n"
"set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})\n"
"set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})\n"
"# Sets global output directory for sub-configurations (msvc, mingw)\n"
"foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})\n"
"    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)\n"
"    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GLOBAL_OUTPUT_PATH})\n"
"    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GLOBAL_OUTPUT_PATH})\n"
"    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GLOBAL_OUTPUT_PATH})\n"
"endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)\n\n"
};


char dependency[] = {
"###############################################################################\n"
"# %s \n"
"###############################################################################\n"
"ExternalProject_Add(\n"
"    %s_External\n"
"    GIT_REPOSITORY \"https://github.com/%s\"\n"
"    GIT_TAG \"%s\"\n"
"    UPDATE_COMMAND \"\"\n"
"    PATCH_COMMAND \"\"\n"
"    SOURCE_DIR \"${CMAKE_SOURCE_DIR}/dep/%s\"\n"
"    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GLOBAL_OUTPUT_PATH}/%s\n"
"    TEST_COMMAND \"\"\n"
"    STAMP_DIR \"${CMAKE_SOURCE_DIR}/dep/\"\n"
")\n"
"ExternalProject_Add_Step(\n"
"    %s_External CopyToBin\n"
"    COMMAND ${CMAKE_COMMAND} -E copy_directory ${GLOBAL_OUTPUT_PATH}/%s/bin ${GLOBAL_OUTPUT_PATH}\n"
"    COMMAND ${CMAKE_COMMAND} -E copy_directory ${GLOBAL_OUTPUT_PATH}/%s/lib ${GLOBAL_OUTPUT_PATH}\n"
"    DEPENDEES install\n"
")\n"
"add_library(%s IMPORTED STATIC GLOBAL)\n"
"set_target_properties(%s PROPERTIES\n"
"        IMPORTED_LOCATION                 \"${CMAKE_BINARY_DIR}/bin/%s/lib/${CMAKE_FIND_LIBRARY_PREFIXES}%s.a\")\n"
"add_dependencies(%s %s_External)\n\n"
};

char footer[] = {
"###############################################################################\n"
"\n"
"include_directories(\n"
"        %s\n"
"        )\n"
"target_link_libraries(${PROJECT_NAME} %s)\n"
"add_dependencies(${PROJECT_NAME} %s)\n"
};

/*
 * Main function for recursing through the graph.
 * NOTE: need to do depth first - then reverse.
 */

char*
recurse_graph(Graph g, FILE* outfile, int level, char* parent, char* grandparent, char* target)
{
    int i,j,k;
    char *repo, *tag, *name = NULL;
    char* deps;

    if (strcmp(g->name,"root")==0) /* Root node - get name of target */
    {
        target = g->nodes[0]->name;
        /*printf("%s\n",target);*/
    }
    else if (g->size==0 ) /* Leaf node - output dependency info! */
    {
        repo = g->name;
        tag = parent;
        name = grandparent;
        /*
        printf("%s\n",name);
        printf("%s\n",repo);
        printf("%s\n",tag);
        */
        fprintf(outfile,dependency,
            name,name,repo,tag,name,name,name,name,name,name,name,name,name,name,name); /* Strewth */

    }

    /* Depth first search through rest... */

    for (i = 0; i < g->size; i++)
    {
        deps = recurse_graph(g->nodes[i], outfile, level + 1, g->name, parent, target);
    }

    return deps;
}

/*
 * Main entry point.  Start here.
 */

int
main(int argc, char **argv)
{
    Graph g;
    FILE *f, *d;
    OgdlParser parser;
    struct argparse argparse;

    char* target;
    char* version;

    /* Set-up argparse info and options. */

    static const char *const usage[] = {
            "cment [file.cment]",
            NULL
    };

    struct argparse_option options[] = {
            OPT_HELP(),
            /* OPT_BOOLEAN('f', "force", &force, "force to do"),  - no options at the mo! */
            OPT_END()
    };

    /* Init and parse the command line. */

    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse,
                      "\nCMent - An absurdly simple dependency manager for CMake.",
                      "\nAlways takes the *.cment file to process.");
    argc = argparse_parse(&argparse, argc, (const char**)argv);

    /* Check we have got the necessary cment file */

    if (argc == 1) /* Solitary, unprocessed option, should be the file! */
    {
        f = fopen(argv[argc-1],"r");
        if (!f) {
            printf ("File %s not found\n",argv[argc-1]);
            exit(1);
        }
    }

    /* Create the parser and parse the file contents. */
   
    parser = OgdlParser_new();
    OgdlParser_parse(parser,f);
    fclose(f); 

    /* Check OK. */

    if (! parser->g || ! parser->g[0]) 
        exit(1);

    /* If so, get the graph. */

    g = parser->g[0];
    Graph_setName(g,"root");
    //g = Graph_get(g,".");

    /* Open the output file. */

    d = fopen("CMent_test.cmake","w");

    /* Write the header. */

    fwrite(header,1,sizeof(header),d);

    /* Recurse through graph depth-first and write out. */

    if (g) {
        recurse_graph(g,d,0,NULL,NULL,NULL);
    }

    /* Tidy up */

    fclose(d);
    OgdlParser_free(parser);
    exit(g?0:1); /* Return error code depending on graph. */
}

