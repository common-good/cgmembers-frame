@REM %1 is start, publish, or finish
git flow feature %1 %2

IF "%1"=="finish" git push