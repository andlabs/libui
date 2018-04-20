	case uiAttributeVerticalForms:
		if (spec->Value != 0) {
			CFAttributedStringSetAttribute(p->mas, range, kCTVerticalFormsAttributeName, kCFBooleanTrue);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassRoman);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassIdeographicCentered);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassIdeographicLow);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassIdeographicHigh);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassHanging);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassMath);
		} else {
			CFAttributedStringSetAttribute(p->mas, range, kCTVerticalFormsAttributeName, kCFBooleanFalse);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassRoman);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassIdeographicCentered);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassIdeographicLow);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassIdeographicHigh);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassHanging);
//			CFAttributedStringSetAttribute(p->mas, range, kCTBaselineClassAttributeName, kCTBaselineClassMath);
		}
		break;
