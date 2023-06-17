#!/bin/sh -e

#git-cache-meta -- simple file meta data caching and applying.
#Simpler than etckeeper, metastore, setgitperms, etc.
#from http://www.kerneltrap.org/mailarchive/git/2009/1/9/4654694
#modified by n1k
# - save all files metadata not only from other users
# - save numeric uid and gid

# 2012-03-05 - added filetime, andris9

GIT_CACHE_META_FILE=.git_cache_meta
case $@ in
    --store)
        git ls-files|while read file; do
            echo "touch -c -m -d \"$(date -r "$file" +'%F %T')\" \"$file\"";
        done > $GIT_CACHE_META_FILE
        ;;
    --apply)
        sh -e $GIT_CACHE_META_FILE
        ;;
    *)
        echo "Usage: $0 --store|--apply"; exit 1
        ;;
esac