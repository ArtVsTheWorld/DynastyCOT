"""Public display policy. No input ratings or gameplay values are changed."""
BANDS=[[(60,'F'),(70,'D'),(80,'C'),(90,'B'),(101,'A')],
       [(60,'F'),(63,'D-'),(67,'D'),(70,'D+'),(73,'C-'),(77,'C'),(80,'C+'),
        (83,'B-'),(87,'B'),(90,'B+'),(93,'A-'),(97,'A'),(101,'A+')],
       [(60,'F'),(70,'D'),(80,'C'),(90,'B'),(95,'A'),(101,'S')],
       [(60,'POOR'),(70,'FAIR'),(80,'GOOD'),(90,'GREAT'),(101,'ELITE')]]
BLIND_SCALE=4
BLIND_LABEL='–'
SCALE_COUNT=5
COLORS={'A':0x1FE840,'B':0xA8D943,'C':0xFFD34D,'D':0xF68C2E,'F':0xF04444,
        'S':0x409EFF,'ELITE':0xBB70EF,'GREAT':0x529FF0,'GOOD':0x66CF75,'FAIR':0xEDB34F,'POOR':0xEA6666}
# Accept older cached labels while new displays use capitals.
for word in ['Poor','Fair','Good','Great','Elite']:COLORS[word]=COLORS[word.upper()]
for tier in 'ABCD':
    for sign in '+-':COLORS[tier+sign]=COLORS[tier]
def label(value,scale=0):return BLIND_LABEL if scale==BLIND_SCALE else next(text for upper,text in BANDS[scale] if value<upper)
def palette(scale=0):
    if scale==BLIND_SCALE:return {}
    order=[text for _,text in BANDS[scale]];result={text:COLORS[text] for text in order}
    for lo in range(len(order)):
        for hi in range(lo+1,len(order)):
            mid=(lo+hi)/2;left=COLORS[order[int(mid)]];right=COLORS[order[int(mid+.5)]]
            color=left if mid.is_integer() else sum(round((((left>>shift)&255)+((right>>shift)&255))/2)<<shift for shift in (16,8,0))
            result[order[lo]+'–'+order[hi]]=color
    return result
