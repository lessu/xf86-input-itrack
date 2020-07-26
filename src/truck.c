
// static void post_button(itrack_t* mt, int button, int new_state);
// static bitmask_t buttons_posted = 0U;
// static void post_gestures(itrack_t *mt)
// {
// 	struct Gestures* gs = &itrack->gs;
// 	int i;
// 	const double delta = timertoms(&gs->dt);

// 	if(itrack->absolute_mode == FALSE){
// 		if (itrack->cfg.scroll_smooth){
// 			/* Never post these buttons in smooth mode. */
// 			CLEARBIT(gs->buttons, MT_BUTTON_WHEEL_UP);
// 			CLEARBIT(gs->buttons, MT_BUTTON_WHEEL_DOWN);
// 			CLEARBIT(gs->buttons, MT_BUTTON_HWHEEL_LEFT);
// 			CLEARBIT(gs->buttons, MT_BUTTON_HWHEEL_RIGHT);
// 		}
// 	}

// 	for (i = 0; i < 32; i++) {
// 		post_button(mt, i, GETBIT(gs->buttons, i));
// 	}

// 	if(itrack->absolute_mode == FALSE){
// 		if (itrack->cfg.scroll_smooth){
// 			ValuatorMask* mask;	mask = itrack->valuator_mask;
// 			valuator_mask_zero(mask);

// 			if (gs->move_dx)
// 				valuator_mask_set_double(mask, 0, gs->move_dx);
// 			if (gs->move_dy)
// 				valuator_mask_set_double(mask, 1, gs->move_dy);
// 			gs->move_dx = gs->move_dy = 0.0;

// 			/* if is any swipe */
// 			if(gs->move_type == GS_SWIPE2 || gs->move_type == GS_SWIPE3 || gs->move_type == GS_SWIPE4){
// 				if(gs->scroll_speed_valid)
// 				{
// 					/* Delta encoded in scroll_speed. */
// 					valuator_mask_set_double(mask, 2, gs->scroll_speed_y);
// 					valuator_mask_set_double(mask, 3, gs->scroll_speed_x);
// 				}
// 				gs->scroll_speed_valid = 0;

// 				/* Start coasting here if needed. */
// 				if(delta != 0.0 && can_start_coasting(mt)){
// 					/* Remove delta component from scroll_speed. */
// 					gs->scroll_speed_x /= delta;
// 					gs->scroll_speed_y /= delta;
// 					mt_timer_start(mt, MT_TIMER_COASTING);
// 				}
// 			}

// 			xf86PostMotionEventM(itrack->local_dev, Relative, mask);
// 		} /* if smooth scroll */
// 		else{
// 			// itrack->absolute_mode == false
// 			if (gs->move_dx != 0.0 || gs->move_dy != 0.0)
// 				xf86PostMotionEvent(itrack->local_dev, 0, 0, 2, (int)gs->move_dx, (int)gs->move_dy);
// 		}
// 	}
// 	else{
// 		/* Give the HW coordinates to Xserver as absolute coordinates, these coordinates
// 		 * are not scaled, this is oke if the touchscreen has the same resolution as the display.
// 		 */
// 		xf86PostMotionEvent(itrack->local_dev, 1, 0, 2,
// 			itrack->state.touch[0].x + get_cap_xmid(&itrack->caps),
// 			itrack->state.touch[0].y + get_cap_ymid(&itrack->caps));
// 	}
// }

// static void post_button(itrack_t* mt, int button, int new_state)
// {
// 	struct Gestures* gs = &itrack->gs;

// 	if (new_state == GETBIT(buttons_posted, button))
// 		return;
// 	if (new_state) {
// 		xf86PostButtonEvent(itrack->local_dev, FALSE, button+1, 1, 0, 0);
// 		LOG_DEBUG_DRIVER("button %d down\n", button+1);
// 	}
// 	else {
// 		xf86PostButtonEvent(itrack->local_dev, FALSE, button+1, 0, 0, 0);
// 		LOG_DEBUG_DRIVER("button %d up\n", button+1);
// 	}
// 	MODBIT(gs->buttons, button, new_state);
// 	MODBIT(buttons_posted, button, new_state);
// }