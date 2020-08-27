if [%1]==[create] PHP create-migration2.php %2
 
@if NOT [%1]==[create] vendor\robmorgan\phinx\bin\phinx "%1" -c config\phinx.json %2 %3 %4 %5 %6 %7 %8
