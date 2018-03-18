	uint32_t vertval;

	case uiAttributeVerticalForms:
		// LONGTERM 8 and/or 8.1 add other methods for vertical text
		op.p = p;
		op.start = start;
		op.end = end;
		vertval = 0;
		if (spec->Value != 0)
			vertval = 1;
		doOpenType("vert", vertval, &op);
		doOpenType("vrt2", vertval, &op);
		doOpenType("vkrn", vertval, &op);
		doOpenType("vrtr", vertval, &op);
		break;
