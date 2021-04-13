<?php
 echo "Hostname: ". @php_uname(n) ."\n";
 if (function_exists( 'shell_exec' )) {
  echo "Hostname: ". @gethostbyname(trim(`hostname`)) . "\n";
 } else {
  echo "Server IP: ". $_SERVER['SERVER_ADDR'] . "\n";
 }
 echo "Platform: ". @php_uname(s) ." ". @php_uname(r) ." ". @php_uname(v) ."\n";
 echo "Architecture: ". @php_uname(m) ."\n";
 echo "Username: ". get_current_user () ." ( UiD: ". getmyuid() .", GiD: ". getmygid() ." )\n";
 echo "Curent Path: ". getcwd () ."\n";
 echo "Server Type: ". $_SERVER['SERVER_SOFTWARE'] . "\n";
 echo "Server Admin: ". $_SERVER['SERVER_ADMIN'] . "\n";
 echo "Server Signature: ". $_SERVER['SERVER_SIGNATURE'] ."\n";
 echo "Server Protocol: ". $_SERVER['SERVER_PROTOCOL'] ."\n";
 echo "Server Mode: ". $_SERVER['GATEWAY_INTERFACE'] ."\n";
?>
