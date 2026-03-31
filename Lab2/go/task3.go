package main

func changeSpacesToUnderscore(input *string) {
	runes := []rune(*input)
	for i, c := range runes {
		if c == ' ' {
			runes[i] = '_'
		}
	}
	*input = string(runes)
}
