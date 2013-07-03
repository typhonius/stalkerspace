<?php
/**
 * @file
 * Definition of geolocation_proximity_views_handler_field_distance.
 */

/**
 * Distance field handler for views.
 */
class geolocation_proximity_views_handler_field_distance extends views_handler_field {

  /**
   * init the handler with necessary data.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    $field_id = str_replace('_distance', '', $this->real_field);
    // $this->additional_fields = array(
    //   $field_id . '_lat_sin',
    //   $field_id . '_lat_cos',
    //   $field_id . '_lng_rad',
    // );
    $this->geolocation_proximity_field_id = $field_id;

    $this->field_alias = $this->options['id'] . '_field';
  }

  /**
   * Information about options for all kinds of purposes will be held here.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['exposed'] = array(
      'default' => TRUE,
      'bool' => TRUE,
    );
    $options['expose'] = array(
      'contains' => array(
        'operator_id' => array('default' => FALSE),
        'label' => array(
          'default' => '',
          'translatable' => TRUE,
        ),
        'use_operator' => array(
          'default' => FALSE,
          'bool' => TRUE,
        ),
        'operator' => array('default' => ''),
        'identifier' => array('default' => ''),
        'required' => array(
          'default' => FALSE,
          'bool' => TRUE,
        ),
        'remember' => array(
          'default' => FALSE,
          'bool' => TRUE,
        ),
        'multiple' => array(
          'default' => FALSE,
          'bool' => TRUE,
        ),
      ),
    );
    return $options;
  }

  /**
   * If a handler has 'extra options' it will get a little settings widget and
   * another form called extra_options.
   */
  function has_extra_options() {
    return TRUE;
  }

  /**
   * Provide defaults for the handler.
   */
  function extra_options(&$option) {
    $option['use_filter'] = FALSE;
    $option['location'] = array(
      'latitude' => 0,
      'longitude' => 0,
    );
  }
  /**
   * Provide a form for setting options.
   */
  function extra_options_form(&$form, &$form_state) {
    $filters = array(
      '' => t(' - Use own values - ')
    );
    foreach ($this->view->get_items('filter') as $filter) {
      if (preg_match('/^' . $this->real_field . '/', $filter['field'])) {
        $filters[$filter['id']] = $filter['ui_name'] ? $filter['ui_name'] . " ({$filter['field']})" : $filter['field'];
      }
    }
    $form['use_filter'] = array(
      '#type' => 'select',
      '#options' => $filters,
      '#title' => t('Use filter'),
      '#default_value' => $this->options['use_filter'],
      '#description' => t('This field requires a location (latitude, longitude) to use as reference for calculating the distance. If there are filters for this same field, the location set for the given filter can be used. If not one needs to be provided.'),
    );

    $form['location'] = array(
      '#tree' => TRUE,
      '#type' => 'fieldset',
      '#title' => t('Default location'),
      '#states' => array(
        // Only show this field when the 'toggle_me' checkbox is enabled.
        'visible' => array(
          '[name="options[use_filter]"]' => array('value' => ''),
        ),
      ),
    );

    $form['location']['latitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Latitude'),
      '#required' => TRUE,
      '#default_value' => $this->options['location']['latitude'],
      '#weight' => 1,
    );
    $form['location']['longitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Longitude'),
      '#required' => TRUE,
      '#default_value' => $this->options['location']['longitude'],
      '#weight' => 2,
    );
  }

  /**
   * Validate the options form.
   */
  function extra_options_validate(&$form, &$form_state) {
    $this->latlng_validate($form['location'], $form_state['values']['options']['location']);
  }

  /**
   * Render our chunk of the exposed handler form when selecting
   */
  function exposed_form(&$form, &$form_state) {

    $field_id = $this->geolocation_proximity_field_id;
    $form[$this->field_alias] = array(
      '#tree' => TRUE,
      '#title' => $this->options['label'],
      '#type' => 'item',
    );

    $form[$this->field_alias]['latitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Latitude'),
      '#required' => TRUE,
      '#default_value' => $this->options['location']['latitude'],
      '#weight' => 1,
    );
    $form[$this->field_alias]['longitude'] = array(
      '#type' => 'textfield',
      '#title' => t('Longitude'),
      '#required' => TRUE,
      '#default_value' => $this->options['location']['longitude'],
      '#weight' => 2,
    );
  }

  /**
   * Validate the exposed handler form
   */
  function exposed_validate(&$form, &$form_state) {

    if (!$this->can_expose()) {
      return;
    }

    $field_id = $this->geolocation_proximity_field_id;
    $this->latlng_validate($form[$this->field_alias], $form_state['values'][$this->field_alias]);
  }

  /**
   * Submit the exposed handler form.
   */
  function exposed_submit(&$form, &$form_state) {
    if (!$this->can_expose()) {
      return;
    }
    $field_id = $this->geolocation_proximity_field_id;
    $this->options['location']['latitude'] = $form_state['values'][$this->field_alias]['latitude'];
    $this->options['location']['longitude'] = $form_state['values'][$this->field_alias]['longitude'];
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
  }

  /**
   * Determine if a handler can be exposed.
   */
  function can_expose() {
    return $this->options['use_filter'] ? FALSE : TRUE;
  }

  /**
   * Called to add the field to a query.
   */
  function query() {
    if (!$this->options['use_filter']) {
      $this->ensure_my_table();
      $this->add_additional_fields();

      $table = $this->table_alias;
      $field_id = $this->geolocation_proximity_field_id;

      // Prepare filter values.
      $filter_lat = $this->options['location']['latitude'];
      $filter_lng = $this->options['location']['longitude'];

      // Prepare field values.
      $field_latsin = "{$table}.{$field_id}_lat_sin";
      $field_latcos = "{$table}.{$field_id}_lat_cos";
      $field_lng= "{$table}.{$field_id}_lng_rad";

      // Build the query.
      $sql = _proximity_sql_fragment($filter_lat, $filter_lng, $field_latsin, $field_latcos, $field_lng);

      $this->query->add_field(NULL, $sql, $this->field_alias);
    }
  }

  /**
   * Render the field.
   *
   * @param array $values
   *   The values retrieved from the database.
   */
  function render($values) {
    if ($this->options['use_filter']) {
      $distance = $values->{$this->options['use_filter'] . '_filter'};
    }
    else {
      $distance = $this->get_value($values);
    }
    return t('@distance km.', array('@distance' => number_format($distance, 2)));
  }
}
