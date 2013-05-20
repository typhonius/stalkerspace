<?php
/**
 * @file
 * Definition of geolocation_proximity_views_handler_filter_distance.
 */

/**
 * General proximity filter for location latitude/longitude.
 */
class geolocation_proximity_views_handler_filter_distance extends views_handler_filter {

  /**
   * init the handler with necessary data.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->field_alias = $this->options['id'] . '_filter';
  }

  /**
   * views_handler_filter::option_definition
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['operator'] = array('default' => '<');
    $options['value']['default'] = array(
      'latitude' => NULL,
      'longitude' => NULL,
      'search_distance' => 100,
      'search_units' => 'kilometers',
    );
    return $options;
  }

  /**
   * Display the filter on the administrative summary
   */
  function admin_summary() {
    return FALSE;
  }

  /**
   * Shortcut to display the value form.
   */
  function value_form(&$form, &$form_state) {

    $form['value'] = array(
      '#tree' => TRUE,
    );

    $form['value']['latitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Latitude'),
      '#required' => TRUE,
      '#default_value' => $this->value['latitude'],
      '#weight' => 1,
    );
    $form['value']['longitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Longitude'),
      '#required' => TRUE,
      '#default_value' => $this->value['longitude'],
      '#weight' => 2,
    );
    $form['value']['search_distance'] = array(
      '#type' => 'textfield',
      '#title' => t('Max distance (km)'),
      '#required' => TRUE,
      '#default_value' => $this->value['search_distance'],
      '#weight' => 3,
    );
  }

  /**
   * Validate the options form.
   */
  function value_validate($form, &$form_state) {
    $this->latlng_validate($form['value'], $form_state['values']['options']['value']);
  }

  function exposed_validate(&$form, &$form_state) {
    $this->latlng_validate($form[$this->options['id']], $form_state['values'][$this->options['id']]);
  }

  /**
   * Validate the latitude and longitude values
   */
  function latlng_validate(&$elements, &$values) {
    switch (TRUE) {
      case !is_numeric($values['latitude']):
        form_error($elements['latitude'], t('Invalid Latitude. Value must be numeric.'));
        break;

      case $values['latitude'] > 90:
      case $values['latitude'] < -90:
        form_error($elements['latitude'], t('Invalid Latitude. Value must be between 90 and -90.'));
        break;
    }

    switch (TRUE) {
      case !is_numeric($values['longitude']):
        form_error($elements['longitude'], t('Invalid Longitude. Value must be numeric.'));
        break;

      case $values['longitude'] > 180:
      case $values['longitude'] < -180:
        form_error($elements['longitude'], t('Invalid Longitude. Value must be between 180 and -180.'));
        break;
    }

    if (!is_numeric($values['search_distance']) || $values['search_distance'] < 0) {
      form_error($elements['search_distance'], t('Invalid Distance. Value must be a positive number.'));
    }

  }

  /**
   * Add this filter to the query.
   */
  function query() {
    if (empty($this->value)) {
      return;
    }

    $value = isset($this->value[0]) ? $this->value[0] : $this->value;

    $this->ensure_my_table();
    $table = $this->table_alias;
    $field_id = str_replace('_distance', '', $this->options['field']);

    // Prepare filter values.
    $filter_distance = $value['search_distance'];
    $filter_lat = $value['latitude'];
    $filter_lng = $value['longitude'];

    // Prepare field values.
    $field_latsin = "{$table}.{$field_id}_lat_sin";
    $field_latcos = "{$table}.{$field_id}_lat_cos";
    $field_lng= "{$table}.{$field_id}_lng_rad";

    // Build the query.
    $sql = _proximity_sql_fragment($filter_lat, $filter_lng, $field_latsin, $field_latcos, $field_lng);

    // We use having to be able to reuse the query on field handlers
    $this->query->add_field(NULL, $sql, $this->field_alias);
    $this->query->add_having($this->options['group'], $this->field_alias, $filter_distance, $this->operator);
  }
}
