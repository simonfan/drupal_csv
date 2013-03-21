<?php

/**
 * Functions defined here are used by both importer and exporter modules.
 */

// creates a Entity_type selector
function csv_common_entity_type_selector($form, &$form_state, $form_values) {
	$form = array();

	$entity_info = entity_get_info();

	// ENTITY INFO
	$entity_types = array_keys($entity_info);

	$entity_types = array_combine($entity_types, $entity_types);
	
    $form['entity_type'] = array(
    	'#title' => 'Entity type to be imported/exported.',
    	'#type' => 'select',
    	'#options' => $entity_types,
    	'#default_value' => (isset($form_values) && isset($form_values['entity_type'])) ? $form_values['entity_type'] : 'node', 
    );

	return $form;
}

// Creates a Entity Type Bundle selector. Requires that $form_values['entity_type'] property exists
function csv_common_entity_bundle_selector($form, &$form_state, $form_values) {
	$form = array();


	// ENTITY BUNDLES
	$entity_type = $form_values['entity_type'];

	$entity_info = entity_get_info($entity_type);

	$bundles = array_keys($entity_info['bundles']);

	$bundles_for_selection = array_combine($bundles, $bundles);

	$form['entity_bundle'] = array(
		'#type' => 'select',
		'#options' => $bundles_for_selection,

		'#title' => t('Select the Entity Type Bundle you wish to import/export'),
		'#default_value' => !empty($form_values['entity_bundle']) ? $form_values['entity_bundle'] : '',
	);


	return $form;
}






// returns a list of properties AND fields, keyed by the machine names and with label names as values
function csv_common_get_field_and_props($entity, $bundle) {

  // now that we know the columns to be mapped FROM,
  // we need to get the fields and properties to map TO. That depends on the node type..

	// get the fields.
  $fields = field_info_instances($entity, $bundle);

  // get the properties
  $properties_info = entity_get_property_info($entity);
  $properties = array_keys( $properties_info['properties'] );

  // this function is used to build the options for the fields select
  function get_field_label($field_instance) {
    return $field_instance['label'];
  }

  // store field options
  $field_options = (array) array_map('get_field_label', $fields);
  $property_options = array_combine($properties, $properties);


  // build the options by merging both property options and field options.
  $map_options = array_merge($field_options, $property_options);

  return $map_options;
}