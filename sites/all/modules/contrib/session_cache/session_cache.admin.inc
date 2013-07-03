<?php

/**
 * @file
 * session_cache.admin.inc
 */

/**
 * Menu callback for admin settings.
 *
 * Contrib modules can add more options by implementing hook_form_alter().
 */
function session_cache_admin_config() {
  $form['session_cache_storage_method'] = array(
    '#type' => 'radios',
    '#title' => t('Where should user session data be stored?'),
    '#default_value' => variable_get('session_cache_storage_method', SESSION_CACHE_STORAGE_SESSION),
    '#options' => array(
      SESSION_CACHE_STORAGE_COOKIE  => t("on the user's computer, in a cookie"),
      SESSION_CACHE_STORAGE_DB_CORE => t("on the server, on core's cache database"),
      SESSION_CACHE_STORAGE_SESSION => t('on the server, in $_SESSION memory')
    ),
    '#description' => t('The first two mechanisms will NOT write to or read from $_SESSION so are generally a good choice when your site uses Varnish or similar page caching engine.')
  );

  $expire_period = (float)variable_get('session_cache_expire_period');
  if ($expire_period <= 0.0) {
    $expire_period = SESSION_CACHE_DEFAULT_EXPIRATION_DAYS;
  }
  $form['session_cache_expire_period'] = array(
    '#type' => 'textfield',
    '#size' => 4,
    '#title' => t('Expiration time for the database cache and cookies created via this module'),
    '#field_suffix' => t('days'),
    '#default_value' => $expire_period,
    '#description' => t('You may use decimals, eg 0.25 equates to 6 hours.<br/>$_SESSION expiration is set via the server configuration. See the <em>sites/default/settings.php</em> file for details.')
  );

  $form['session_cache_use_uid_as_sid'] = array(
    '#type' => 'checkbox',
    '#title' => t("Remember the user's session from one browser to the next"),
    '#default_value' => variable_get('session_cache_use_uid_as_sid', FALSE),
    '#description' => t('Applies to authenticated users only and does not work for the cookie and $_SESSION storage mechanisms.')
  );

  return system_settings_form($form);
}
