(StackView Name="Quote" class="card stack-card quote-card"
  (TextBlock Name="QuoteMark" class="quote-mark" Text="“"
    ForegroundColor=(bind "DataContext/PrimaryColor"))
  (TextBlock Name="QuoteText" class="quote-text" Text=(bind "DataContext/Body"))
  (StackView Name="QuoteAuthor" class="quote-author"
    (TextBlock Name="QuoteAuthorName" class="quote-author-name" Text=(bind "DataContext/Title"))
    (TextBlock Name="QuoteAuthorRole" class="quote-author-role" Text=(bind "DataContext/Subtitle"))))
