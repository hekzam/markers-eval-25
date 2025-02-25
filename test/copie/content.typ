#import "../original.typ": rect-box, gen-copies
#let bcol = json("./input-fill-colors.json")
#let f-color = white
#let copy-two-pages = int(sys.inputs.at("copy-two-pages", default: "0")) == 1

#let copy-content = [
  #set align(left+top)

  = Exercise 1
  #rect-box("b0", 5cm, 1cm, fill-color: color.rgb(bcol.at("b0")))

  #box(rect-box("b0i", 8cm, 2.5cm, fill-color: color.rgb(bcol.at("b0i"))))

  #rect-box("b1", 100%, 1cm, fill-color: color.rgb(bcol.at("b1")))

  #if copy-two-pages {
    pagebreak()
  }

  = Exercise 2
  #rect-box("b2", 5cm, 1cm, fill-color: color.rgb(bcol.at("b2")))

  #box(rect-box("b2i", 8cm, 2.5cm, fill-color: color.rgb(bcol.at("b2i"))))

  = Exercise 3
  This exercise should be at the bottom of its page.
  #rect-box("b3", 100%, 1cm, fill-color: color.rgb(bcol.at("b3")))

  = Exercice 4: Code Imaginaire
  This is placeholder code: 1234567890

  = Exercice 5: Autres placeholders
  Here is some dummy code:
  #box("def placeholder():\n    print(\"Placeholder function executed\")\n    return \"result\"\n\nprint(placeholder())")
]

#let n = int(sys.inputs.at("nb-copies", default: "1"))
#let d = int(sys.inputs.at("duplex-printing", default: "0")) == 1

#gen-copies("basic", copy-content, nb-copies: n, duplex-printing: d)