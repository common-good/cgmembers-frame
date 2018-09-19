if exist 0bootstrap.inc (
  ren bootstrap.inc 3bootstrap.inc
  ren 0bootstrap.inc bootstrap.inc
) else (
  ren bootstrap.inc 0bootstrap.inc
  ren 3bootstrap.inc bootstrap.inc
)