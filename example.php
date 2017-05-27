 <?php
/*
    sudo cp templater.so /usr/lib/php5/20060613+lfs/
    sudo echo 'zend_extension=templater.so' > /etc/php5/cli/conf.d/99-template.ini    
*/
 
$template = "Some string @macros1 and @macros2 ... @macros1 ... string @macros1 and @macros2 ... @macros1 ...";

$tpl = new Templater($template);
$tpl->set("@macros1", "VALUE 1");
$tpl->set("@macros2", "VALUE 2");

echo $tpl->render();

echo "\n";
