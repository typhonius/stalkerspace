<?php
/**
 * @file
 * ip_geoloc.session.inc
 *
 * Implements via Session Cache API one of several storage mechanisms for the
 * user's changing location details.
 */

/**
 *
 * @param type $name
 *   if FALSE all IP Geolocation session variables are deleted;
 *   if NULL the array of all IP Geolocation session variables is returned
 * @param type $value, if
 * @return type
 */
function _ip_geoloc_set_session_value($name, $value = NULL) {

  if (!module_exists('session_cache')) {
    if ($name === FALSE) {
      unset($_SESSION['ip_geoloc']);
    }
    elseif (isset($name)) {
      $_SESSION['ip_geoloc'][$name] = $value;
    }
    return isset($_SESSION) && isset($_SESSION['ip_geoloc']) ? $_SESSION['ip_geoloc'] : NULL;
  }

  if ($name === FALSE) {
    session_cache_set('ip_geoloc', NULL);
    return NULL;
  }

  // Efficiency: for multiple get's during the same request avoid having to go
  // to the storage mechanism each time.
  $variables = &drupal_static(__FUNCTION__, array());
  if (empty($variables)) {
    $variables = session_cache_get('ip_geoloc');
  }
  if (isset($name)) {
    $variables[$name] = $value;
    session_cache_set('ip_geoloc', $variables);
  }
  return $variables;
}

function _ip_geoloc_get_session_value($name = NULL) {
  // Get all variables, then return the one requested or all if $name==null
  $variables = _ip_geoloc_set_session_value(NULL);
  return empty($name) ? $variables : (isset($variables[$name]) ? $variables[$name] : NULL);
}
