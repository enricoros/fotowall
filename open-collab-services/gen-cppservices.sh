function printa {
echo "QString $1() const {return m_$1;}"
echo "void set$1(const QString & $1) {m_$1 = $1;}"
echo
}

printa id
printa status
printa contentid
printa user
printa changed
printa name
printa description
printa answer
printa comments
printa detailpage
