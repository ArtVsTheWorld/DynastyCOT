"""Dollar-prefix notation for the existing independently specified test amounts."""
def usd(label):
    # Preserve the source test's slash/space layout; each money amount gets $.
    parts=[]
    for part in label.split('/'):
        text=part.strip()
        parts.append(part if not text else part[:len(part)-len(part.lstrip())]+'$'+text+part[len(part.rstrip()):])
    return '/'.join(parts)
