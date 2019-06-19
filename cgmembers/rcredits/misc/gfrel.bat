@REM %1 is start or finish
git flow release %1 "%2"

IF "%1"=="start" (
  SED -i -r "s/'R_VERSION', '[0-9\.a-z]+'/'R_VERSION', '%2'/" cgmembers/rcredits/defs.inc
  GIT add -A
  GIT commit -m "v%2"
  git flow release publish "%2"
)
