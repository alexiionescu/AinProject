#pragma once

ain_result_t ain_child_process();
ain_result_t ain_parent_process_ux();
ain_result_t ain_parent_process(ain_process_t* child);
ain_result_t ain_controller_process();
int ain_is_debug_child();
int ain_is_controller();
int ain_is_test_unit();
ain_result_t ain_test_unit_process();

			
			
