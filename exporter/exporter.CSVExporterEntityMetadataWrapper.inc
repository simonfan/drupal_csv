<?php


class CSVExporterEntityMetadataWrapper {

	// an array with the extra available data that should be available
	// for special field types
	public $extra_available_data = array(
		'image' => array(
			'filename',
			'uri',
			'filemime',
			'filesize',
		),
		'entityreference' => array(
			'title',
		),
		'taxonomy_term_reference' => array(
			'term_name',
		),
	);

	public function __construct($entity_type, $entity = NULL) {
		$this->wrapper = entity_metadata_wrapper($entity_type, $entity);
	}


	public function field_available_data($field_name) {
		// get field info
		$finfo = field_info_field($field_name);
		// if field_info_field returns NULL, that menas the $field_name refer not to a field, but to a property
		$finfo = isset($finfo) ? $finfo : array( 'type' => 'property', 'columns' => array(), 'cardinality' => 1 );

		$field_columns = $finfo['columns'];

		$field_columns = array_keys($field_columns);

		// extra variables
		$field_type = $finfo['type'];
		$extra = isset($this->extra_available_data[ $field_type ]) ? $this->extra_available_data[ $field_type ] : array();

		// if the field is a special one, add 
		// other data possibilities to the available_data array.
		$data_names = array_merge(
			array('default'),		// the default value, to be accessed through EntityMetadataWrapper->value()
			$field_columns, 		// the field's columns available values
			$extra  				// some extra artificially available values. These we will have to manually build in the next phase if checked.
		);

		
		return $data_names;
	}



	public function get_field_data_value($field_data_name) {
		// field_name may be a simple field_name or a field_name:data_name
		// try to explode the $field_data_name with ':'
		$pieces = explode(':', $field_data_name);

		$field_name = isset( $pieces[0] ) ? trim($pieces[0]) : FALSE;
		$data_name = isset( $pieces[1] ) ? trim($pieces[1]) : FALSE;


		// get field info
		$finfo = field_info_field($field_name);
		// if field_info_field returns NULL, that menas the $field_name refer not to a field, but to a property
		$finfo = isset($finfo) ? $finfo : array( 'type' => 'property', 'columns' => array(), 'cardinality' => 1 );


		$field_is_single = $finfo['cardinality'] == 1;

		$data_array = array(
			'field_name' => $field_name,
			'data_name' => $data_name,
			'is_single' => $field_is_single,
			'subvalue_delimiter' => ',',
		);

		if ($field_name && $data_name) {

			return $this->extract_data($data_array);

		} else if ($field_name) {
			// this is the default value returned by the wrapper.
			return $this->extract_default_values($data_array);
		}
	}

	/////////////////////////
	/// SPECIAL PROCESSORS //
	/////////////////////////

	// function gets the field's default values
	public function extract_default_values($d) {
		/**
		 * d: field_name, is_single, subvalue_delimiter
		 */
		$field_name = $d['field_name'];
		$is_single = $d['is_single'];
		$delimiter = $d['subvalue_delimiter'];

		$ans;

		if ($is_single) {

			$ans = $this->wrapper->{$field_name}->value();

		} else {

			$list = $this->wrapper->{$field_name}->value();
			$ans = array();

			foreach ($list as $key => $value) {
				array_push($ans, $value);
			}

			$ans = implode($delimiter, $ans);

		}

		return $ans;
	}


	// function processes 
	public function extract_data($d) {
		/**
		 * d: field_name, data_name, is_single, subvalue_delimiter
		 */
		$field_name = $d['field_name'];
		$data_name = $d['data_name'];
		$is_single = $d['is_single'];
		$delimiter = $d['subvalue_delimiter'];

		// the var that holds the answer
		$ans;


		//////////////////
		// $w->value() ///
		//////////////////
		if ($is_single) {
			$item = $this->wrapper->{$field_name}->value();

			$ans = $this->extract_data_from_single($item, $data_name);

		} else {

			$list = $this->wrapper->{$field_name}->value();

			$ans = $this->extract_data_from_list($list, $data_name, $delimiter);
		}


		////////////////////
		//// $w->raw() /////
		////////////////////
		if ( ( $is_single && empty($ans) ) || (!$is_single && is_array($ans) && empty($ans) ) ) {
			// if the answer is empty, try using the ->raw() value instead of ->value();

			if ($is_single) {
				$item = $this->wrapper->{$field_name}->raw();

				$ans = $this->extract_data_from_single($item, $data_name);
			} else {
				$list = $this->wrapper->{$field_name}->raw();

				$ans = $this->extract_data_from_list($list, $data_name, $delimiter);

			}

		}

		return $ans;
	}

	public function extract_data_from_single($item, $data_name) {
		if ( is_object($item) ) {
			$ans = isset($item->{$data_name}) ? $item->{$data_name} : NULL;
		} elseif ( is_array($item) ) {
			$ans = isset($item[$data_name]) ? $item[$data_name] : NULL;
		}

		return $ans;
	}


	public function extract_data_from_list($list, $data_name, $delimiter) {
		$ans = array();

		foreach($list AS $key => $item) {
			$data = $this->extract_data_from_single($item, $data_name);

			array_push($ans, $data);
		}

		$ans = array_filter($ans);
		$ans = implode($delimiter, $ans);

		return $ans;
	}


	public function export($field_data_names) {
		// field_data_names is an array with multiple field_data names,
		// in the correct order
		$field_data_values = array_map(
			array($this, 'get_field_data_value'), 
			$field_data_names
		);


		$return = array_combine($field_data_names, $field_data_values);

		return $return;
	}
}