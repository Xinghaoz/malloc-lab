/*	If the deleting node has two more brothers 
	 *	bp -> bro -> left = bp -> left;
	 *	bp -> left -> bro = bp -> bro;
	 */

    if (GET_BROTHER(GET_LEFT(bp)) == bp) {
        LEFT_PTR(GET_BROTHER(bp), GET_LEFT(bp));
		BROTHER_PTR(GET_LEFT(bp), GET_BROTHER(bp));
    }
	/* If the deleting node doesn't have brothers */
	else if(GET_BROTHER(bp) == MM_NULL) {
		if (NULL == GET_RIGHT(bp) && NULL == GET_LEFT(bp)) {
			if (GET_PARENT(bp) == NULL) {
				root = NULL;
			} else {
				if (GET_LEFT(GET_PARENT(bp)) == bp) {
					GET_LEFT(GET_PARENT(bp)) = NULL;
				} else {
					GET_RIGHT(GET_PARENT(bp)) = NULL;
				}
			}
		} else if (NULL != GET_LEFT(bp) && NULL == GET_RIGHT(bp)) {
			if (GET_PARENT(bp) == NULL) {
				root = GET_LEFT(bp);
			} else {
				if (GET_LEFT(GET_PARENT(bp)) == bp) {
					GET_LEFT(GET_PARENT(bp)) = GET_LEFT(bp);
				} else {
					GET_RIGHT(GET_PARENT(bp)) = GET_LEFT(bp);
				}
			}
		} else if (NULL == GET_LEFT(bp) && NULL != GET_RIGHT(bp)) {
			if (GET_PARENT(bp) == NULL) {
				root = GET_RIGHT(bp);
			} else {
				if (GET_LEFT(GET_PARENT(bp)) == bp) {
					GET_LEFT(GET_PARENT(bp)) = GET_RIGHT(bp);
				} else {
					GET_RIGHT(GET_PARENT(bp)) = GET_RIGHT(bp);
				}
			}
		} else {
			char * left_node = bp;
			char * parent_node = GET_PARENT(bp);
			while (GET_RIGHT(left_node) != NULL) {
				parent_node = left_node;
				left_node = GET_RIGHT(left_node);
			}
			unsigned int swap_value = GET(left_node);
			PUT(left_node, GET(bp));
			PUT(bp, swap_value);
			if (GET_LEFT(GET_PARENT(bp)) == bp) {
				GET_LEFT(GET_PARENT(bp)) = NULL;
			} else {
				GET_RIGHT(GET_PARENT(bp)) = NULL;
			}
		}

		
	}