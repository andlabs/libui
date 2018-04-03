	PangoGravity gravity;

	case uiAttributeVerticalForms:
		gravity = PANGO_GRAVITY_SOUTH;
		if (spec->Value != 0)
			gravity = PANGO_GRAVITY_EAST;
		addattr(p, start, end,
			pango_attr_gravity_new(gravity));
		break;
