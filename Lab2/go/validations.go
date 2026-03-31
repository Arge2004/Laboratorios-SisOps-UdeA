package main

import "unicode"

func cleanValidate(input *string) bool {
	runes := []rune(*input)
	result := make([]rune, 0, len(runes))
	for _, c := range runes {
		if unicode.IsLetter(c) || unicode.IsSpace(c) {
			result = append(result, c)
		}
	}
	*input = string(result)
	return len(result) > 0
}
