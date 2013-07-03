<?php

/**
 * Implementation of hook_views_plugins().
 */
function geolocation_views_views_plugins() {
  return array(
    'module' => 'geolocation_views',
    'style' => array(
      'google_map' => array(
        'title' => t('Google Map (Geolocation Views)'),
        'handler' => 'geolocation_views_plugin_style_google_map',
        'theme' => 'geolocation_views_map',
        'type' => 'normal',
        'uses row plugin' => TRUE,
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'uses grouping' => FALSE,
        'even empty' => TRUE,
      ),
    ),
  );
}

