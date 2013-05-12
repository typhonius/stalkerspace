===========================================================
OVERVIEW
===========================================================
NOTE: THIS is a pre-alpha release. It works, but is not fully
tested and there is currently no upgrade path from 1.x
versions.

Subdomain automatically generates subdomains for your site
and places site content onto those subdomains.

Currently, it supports 2 modes (more soon):

 OG - place organic groups & content onto subdomains
  EXAMPLE: A group named "Pizza lovers" & content would be located at:
  http://pizza-lowers.example.com

 Node Author - place node author & content onto subdomains
  EXAMPLE: A user named "Sayuko" and her content would be located at:
  http://sayuko.example.com

===========================================================
INSTALL
===========================================================
STEP 1: Set the $cookie_domain variable in your settings.php file
 to your site's domain (e.g. $cookie_domain = ".example.com")

STEP 2: Enable wildcard DNS on your DNS hosting provider 
 (e.g. *.example.com)

STEP 3: Configure your webserver for wildcard virtual hosts.
 (HINT for apache: ServerAlias *.example.com)

STEP 4: Enable this module and configure
