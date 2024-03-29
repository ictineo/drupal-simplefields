<?php

/**
 * @file
 * Administrative interface for custom field type creation.
 */

/**
 * Menu callback; lists all defined fields for quick reference.
 */
function simple_fields_manager_field_condition($form, &$form_state, $instance) {
  $form = array();
  // Token form conditional_fields module .admin.inc file
  $entity_type = variable_get('simple_fields_entity_type','');
  $bundle_name = variable_get('simple_fields_entity_bundle','');
  $location_field = variable_get('simple_fields_location_field','');
  $exclude_fields = variable_get('simple_fields_exclude_fields','');
  $instance_name = field_read_instances(array('id' => $instance));
  $instance_name = array_shift($instance_name);
  $instance_label = $instance_name['label'];
  $instance_name = $instance_name['field_name'];
  // Also exclude the location field and the field we are editing
  $exclude_fields[$location_field] = $location_field;
  $exclude_fields[$instance_name] = $instance_name;
  $instances = field_info_instances($entity_type, $bundle_name);
  $aviable_conditionals = array();
  // Excluding fields
  foreach ($instances as $field) {
    if(in_array($field['field_name'], $exclude_fields)) { continue; }
    $fields[$field['id']] = check_plain($field['label'] . ' (' . $field['field_name'] . ')');
    $aviable_conditionals[$field['field_name']] = $field['id'];
  }
  // Load current dependences to set the default value
  $conditions = conditional_fields_load_dependencies($entity_type, $bundle_name);
  $default_value = 'none';
  $current_cond = '';
  if(isset($conditions['dependents'][$instance_name])) {
    foreach ( $conditions['dependents'][$instance_name] as $id => $cond) {
      /** We assume there is only one dependency in this instance **/
      if(array_key_exists($cond['dependee'],$aviable_conditionals)) {
          $default_value = $aviable_conditionals[$cond['dependee']];
          $current_cond = $id;
      }
    }
  }
  $field[$location_field]['und']['#title'] = t('Set location condition for: ') . $instance_label;
  $fields['none'] = t(' - No condition - ');
  drupal_set_title(t('Set field dependence for: ') . $instance_label);
  asort($fields);
  $form['current_dep'] = array(
    '#type' => 'hidden',
    '#value' => $current_cond,
  );
  $form['depenent'] = array(
      '#type' => 'hidden',
      '#value' => $instance,
    );
  $form['dependee'] = array(
      '#type' => 'select',
      '#title' => t('Set field dependence for: ') . $instance_label,
  //    '#title_display' => 'invisible',
      '#description' => t('Dependee'),
      '#options' => $fields,
      '#default_value' => $default_value,
    );
  $form =  system_settings_form($form);
  $form['#submit'][] = 'simple_fields_manager_field_condition_submit';
  return $form;
}

function simple_fields_manager_field_condition_submit($form, &$form_state) {
  // TODO if the condition is the same not delete and create, better ignore
  if($form_state['values']['current_dep'] != '') {
    conditional_fields_dependency_delete(array($form_state['values']['current_dep']));
  }
  if($form_state['values']['dependee'] != 'none') {
    $options = array();
    // Ajustament de la condició ja que ara hi ha configurats
    // els valors per defecte
    $options['state'] =  'visible';
    //$options['state'] =  'invisible';
    $options['condition'] =  'value';
    $options['grouping'] =  'AND';
    $options['effect'] =  'slide';
    $options['effect_options'] = array('speed' =>  '400');
    $options['selector'] =  '';
    $options['values_set'] =  '1';
    $options['value_form'] =  NULL;
    $options['value'] = array(0=>array('value' => 1));
    conditional_fields_dependency_insert($form_state['values']['dependee'],$form_state['values']['depenent'], $options);
  }
  $form_state['redirect'] = 'admin/config/content/simplefields';
}

function simple_fields_manager_location_condition($form, &$form_state, $instance) {
  // Get configuration values
  $entity_type = variable_get('simple_fields_entity_type','');
  $bundle_name = variable_get('simple_fields_entity_bundle','');
  $location_field = variable_get('simple_fields_location_field','');
  $location_field_w = variable_get('simple_fields_location_field_widget','');
  $location_field_w = 'field_ajn'; //variable_get('simple_fields_location_field_widget','');
  // Prepare the location field
  $dependee = field_info_field($location_field);
  $dependee_w = field_info_field($location_field_w);
  $instance_name = field_read_instances(array('id' => $instance));
  $instance_name = array_shift($instance_name);
  $instance_label = $instance_name['label'];
  $instance_name = $instance_name['field_name'];
  $instances = field_read_instances(array('field_id' => $dependee['id'], 'entity_type' => $entity_type, 'bundle' => $bundle_name));
  $instances_w = field_read_instances(array('field_id' => $dependee_w['id'], 'entity_type' => $entity_type, 'bundle' => $bundle_name));
  $dependee_instance = array_shift($instances);
  $dependee_instance_w = array_shift($instances_w);
  $dependee_instance['default_value_function'] = '';
  $dependee_instance_w['default_value_function'] = '';
  $dummy_form = array('#parents' => array());
  $dependee_instance['widget'] = $dependee_instance_w['widget'];
  $field = field_default_form($entity_type, NULL, $dependee, $dependee_instance, LANGUAGE_NONE, array(), $dummy_form, $form_state);
  $conditions = conditional_fields_load_dependencies($entity_type, $bundle_name);
                                                                                                                                     
  // Only one value
  unset($field[$location_field][$field[$location_field]['#language']]['add_more']);
  foreach ( $conditions['dependees'][$location_field] as $id => $cond) {
    /** We assume there is only one dependency in this instance **/
    if($cond['dependent'] == $instance_name) {
      // We are getting the id of the condition we whant to change 
      foreach($cond['options']['value'] as $k => $v) {
        //$field[$location_field]['und']['#default_value'][$v['tid']] = $v;
        if(is_numeric($v['tid'])) {
          $field[$location_field][$field[$location_field]['#language']][0]['tid']['#default_value'] = $v['tid'];
        }
      }
    }
  }
  $field[$location_field]['und']['#title'] = t('Set location condition for: ') . $instance_label;
  drupal_set_title(t('Set location condition for: ') . $instance_label);
  $field['depenent'] = array(
      '#type' => 'hidden',
      '#value' => $instance,
    );
  $field['dependee'] = array(
      '#type' => 'hidden',
      '#value' => $dependee_instance['id'],
    );
  $field['#submit'][] = 'simple_fields_manager_location_condition_submit';
//  $field[$location_field]['und']['#type'] = 'select';
//  $field[$location_field]['und']['#multiple'] = TRUE;
  $field[$location_field]['und']['#max_choices'] = -1;
  $field[$location_field]['und']['#leaves_only'] = 0;
  $field[$location_field]['und']['#cascading_selection'] = 1;
  
  return system_settings_form($field);
}
function simple_fields_manager_location_condition_submit($form, &$form_state) {
  /**
   * Prepare data
   */
  $entity_type = variable_get('simple_fields_entity_type','');
  $bundle_name = variable_get('simple_fields_entity_bundle','');
  $location_field = variable_get('simple_fields_location_field','');
  $instance_name = field_read_instances(array('id' => $form_state['values']['depenent']));
  $instance_name = array_shift($instance_name);
  $instance_name = $instance_name['field_name'];
  /** 
   * Clear exsistent conditions
   */
  $conditions = conditional_fields_load_dependencies($entity_type, $bundle_name);
  foreach ( $conditions['dependees'][$location_field] as $id => $cond) {
    if($cond['dependent'] == $instance_name) {
      conditional_fields_dependency_delete(array($id));
    }
  }
  /**
   * Create neu conditions
   */
  $options = array();
  $options['state'] =  'visible';
  $options['condition'] =  'value';
  $options['grouping'] =  'AND';
  $options['effect'] =  'slide';
  $options['effect_options'] = array('speed' =>  '400');
  $options['selector'] =  '';
  $options['values_set'] =  '1';
  $options['value_form'] =  NULL;
  // View node options default values alteretd
  $options['element_view'] =  Array(
      '1' => 0,
      '2' => 0,
      '3' => 0,
      '4' => 0,
      '5' => 0,
    );
  foreach($form_state['values'][$location_field]['und'] as $k => $v) {
    $options['value'] = array(0=> $v);
    $options['value_form'] = array(0 => $v['tid']);
    //dsm($options);
    if($v['tid'] != '0') {
      conditional_fields_dependency_insert($form_state['values']['dependee'],$form_state['values']['depenent'], $options);
    }
  }
  drupal_set_message(t('condition updated'));
  $form_state['redirect'] = 'admin/config/content/simplefields';
} 

/**
 * Form constructor for the field instance settings form.
 *
 *  Taken from fields_ui.admin.inc
 * @see field_ui_field_edit_form_validate()
 * @see field_ui_field_edit_form_submit()
 * @ingroup forms
 */
function simple_fields_field_edit_label_form($form, &$form_state, $instance) {
  // Prepare the location field
  $entity_type = variable_get('simple_fields_entity_type','');
  $bundle_name = variable_get('simple_fields_entity_bundle','');
  $instance_name = field_read_instances(array('id' => $instance));
  $instance_name = array_shift($instance_name);
  $instance_label = $instance_name['label'];
  $weight = $instance_name['widget']['weight'];
  $instance_name = $instance_name['field_name'];
  drupal_set_title($instance_label);
  $field = field_info_field($instance_name);
  if (!empty($field['locked'])) {
    $form['locked'] = array(
      '#markup' => t('The field %field is locked and cannot be edited.', array('%field' => $instance['label'])),
    );
    return $form;
  }
  $form['instance']['field_label'] = array(
    '#type' => 'textfield',
    '#title' => t('New label for: ') . $instance_label . '(' . $instance_name .').',
    '#default_value' => !empty($instance_label) ? $instance_label : $instance_name,
    '#required' => TRUE,
    '#size' => 60,
    '#maxlength' => 260,
    '#weight' => -20,
  );
  // Build the non-configurable instance values.
  $form['instance']['field_name'] = array(
    '#type' => 'value',
    '#value' => $instance_name,
  );
  $form['instance']['weight'] = array(
    '#type' => 'value',
    '#value' => $weight,
  );
  $form['instance']['entity_type'] = array(
    '#type' => 'value',
    '#value' => $entity_type,
  );
  $form['instance']['bundle'] = array(
    '#type' => 'value',
    '#value' => $bundle_name,
  );
  $form =  system_settings_form($form);
  $form['#submit'][] = 'simple_fields_field_edit_label_form_submit';
  return $form;
}
function simple_fields_field_edit_label_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $instance = array(
    'field_name' => $values['field_name'],
    'entity_type' => $values['entity_type'],
    'bundle' => $values['bundle'],
    'label' => $values['field_label'],
    'type' => variable_get('simple_fields_field_type',''),
    'widget' => array(
      'type' => variable_get('simple_fields_field_widget', ''),
      'weight' => $values['weight'],
      'settings' => array('display_label' => 1),
    ),
    );
  field_update_instance($instance);
  $form_state['redirect'] = 'admin/config/content/simplefields';
}

function simple_fields_manager_admin($form, &$form_state) {
  $entity_type = variable_get('simple_fields_entity_type','');
  $bundle_name = variable_get('simple_fields_entity_bundle','');
  $instances = field_info_instances($entity_type, $bundle_name);
  $field_list = array();
  $exclude_fields = array(variable_get('simple_fields_location_field',''));
  foreach ($instances as $field) {
    if(in_array($field['field_name'], $exclude_fields)) { continue; }
    $field_list[$field['field_name']] = check_plain($field['label'] . ' (' . $field['field_name'] . ')');
  }
  require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'field_ui') . '/field_ui.admin.inc'; 
  $field_type_options = field_ui_field_type_options();
  $widget_type_options = field_ui_widget_type_options(NULL, TRUE);
  $form['simple_fields_entity_type'] = array ( // Select the entity to work with
    '#type' => 'textfield',
    '#title' => t('Select the entity type to work with'),
    '#default_value' => variable_get('simple_fields_entity_type', ''),
    // TODO CARREGAR LA LLISTA DE VALORS POSIBLES VIA API);
    );
  $form['simple_fields_entity_bundle'] = array ( //Select the entity bundle to work with
    '#type' => 'textfield',
    '#title' => t('Select the entity type to work with'),
    '#default_value' => variable_get('simple_fields_entity_bundle', ''),
    // TODO CARREGAR LA LLISTA DE VALORS POSIBLES VIA API);
    );
  $form['simple_fields_location_field'] = array (
    '#type' => 'textfield',
    '#title' => t('Location field name'),
    '#default_value' => variable_get('simple_fields_location_field',''),
    );
  $form['simple_fields_location_field_widget'] = array (
    '#type' => 'textfield',
    '#title' => t('Location field name widget working'),
    '#default_value' => variable_get('simple_fields_location_field_widget',''),
    );
  $form['simple_fields_exclude_fields'] = array (
    '#type' => 'select',
    '#title' => t('Fields to be excluded for simplefields manager'),
    '#default_value' => variable_get('simple_fields_exclude_fields',''),
    '#multiple' => TRUE,
    '#size' => 6,
    '#options' => $field_list,
    );
  $form['simple_fields_noconf_fields'] = array (
    '#type' => 'select',
    '#title' => t('Fields without configuration options for simplefields manager'),
    '#default_value' => variable_get('simple_fields_noconf_fields',''),
    '#multiple' => TRUE,
    '#size' => 6,
    '#options' => $field_list,
    );
 
  $form['simple_fields_field_type'] = array (
        '#title' => t('Type of the data to store'),
        '#type' => 'select',
        '#title' => t('Type of new field'),
        '#title_display' => 'invisible',
        '#options' => $field_type_options,
        '#empty_option' => t('- Select a field type -'),
        '#description' => t('Type of data to store.'),
        '#attributes' => array('class' => array('field-type-select')),
        '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
        '#default_value' => variable_get('simple_fields_field_type' ,''),
    );
  $form['simple_fields_field_widget'] = array (
        '#title' => t('Form element to edit the data'),
        '#type' => 'select',
        '#title' => t('Widget for new field'),
        '#title_display' => 'invisible',
        '#options' => $widget_type_options,
        '#empty_option' => t('- Select a widget -'),
        '#description' => t('Form element to edit the data.'),
        '#attributes' => array('class' => array('widget-type-select')),
        '#cell_attributes' => array('colspan' => 3),
        '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
        '#default_value' => variable_get('simple_fields_field_widget', ''),
      );
 
  return system_settings_form($form);
}

