@REM %1 is start, publish, or finish
git flow hotfix %1 %2

IF %1=="finish" git push origin HEAD