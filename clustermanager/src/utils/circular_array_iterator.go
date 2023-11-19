package utils

type CircularArrayIterator[T any] struct {
	Array []T

	nextIndex int
	backIndex int
}

func (receiver *CircularArrayIterator[T]) WithStartingIndex(startIndex int) {
	receiver.backIndex = startIndex
	receiver.nextIndex = startIndex
}

func (receiver *CircularArrayIterator[T]) Next() T {
	newNextIndex := receiver.nextIndex + 1

	if newNextIndex >= len(receiver.Array) {
		newNextIndex = 0
	}

	receiver.nextIndex = newNextIndex

	return receiver.Array[newNextIndex]
}

func (receiver *CircularArrayIterator[T]) Back() T {
	newBackIndex := receiver.backIndex - 1

	if newBackIndex < 0 {
		newBackIndex = len(receiver.Array) - 1
	}

	receiver.backIndex = newBackIndex

	return receiver.Array[newBackIndex]
}
