
[HTTP Headers]

<?php

foreach( getallheaders() as $headerKey => $headerVal ){

	print( '* "' . ${headerKey} . '": "' . ${headerVal} . "\"\n" ) ;

}

?>

[POST Parameters]

<?php

foreach( $_POST as $postKey => $postVal ){

	print( '* "' . ${postKey} . '":  "' . ${postVal} . "\"\n" ) ;

}

?>

