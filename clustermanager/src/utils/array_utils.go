package utils

import "sort"

func InsertIntoSortedArray(array []interface{},
	toInsert interface{},
	comparator func(i int) bool) []interface{} {

	index := sort.Search(len(array), comparator)

	if index+1 == len(array) {
		return append(array, toInsert)
	}

	array = append(array, 0)

	nextValue := array[index]
	array[index] = toInsert

	for i := index + 1; i < len(array); i++ {
		lenArrayAux := nextValue
		nextValue = array[i]
		array[i] = lenArrayAux
	}

	return array
}
